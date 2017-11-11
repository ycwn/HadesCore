

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/blob.h"
#include "core/logger.h"
#include "core/archive.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/pixelformat.h"
#include "gr/texture.h"
#include "gr/dds.h"


#define CLAMP_MODE(x) (                                                                     \
	((x) == GR_TEXCOORD_REPEAT)?               VK_SAMPLER_ADDRESS_MODE_REPEAT:          \
	((x) == GR_TEXCOORD_MIRRORED_REPEAT)?      VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: \
	((x) == GR_TEXCOORD_CLAMP_TO_EDGE)?        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:   \
	((x) == GR_TEXCOORD_CLAMP_TO_BORDER)?      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER: \
	((x) == GR_TEXCOORD_MIRROR_CLAMP_TO_EDGE)? VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE: VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)


static list      textures       = LIST_INIT(&textures, NULL);
static int       default_filter = GR_TEXTURE_FILTER_BILINEAR;
static graphics *gfx            = NULL;


const struct mipmap_filter {

	int minification;
	int magnification;
	int mipmapping;

} mipmap_filter_type[] = {

	{ VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST }, // No filtering
	{ VK_FILTER_LINEAR,  VK_FILTER_LINEAR,  VK_SAMPLER_MIPMAP_MODE_NEAREST }, // Linear
	{ VK_FILTER_LINEAR,  VK_FILTER_LINEAR,  VK_SAMPLER_MIPMAP_MODE_NEAREST }, // Bilinear
	{ VK_FILTER_LINEAR,  VK_FILTER_LINEAR,  VK_SAMPLER_MIPMAP_MODE_LINEAR  }, // Trilinear

};


static void reset(  gr_texture *t);
static void destroy(gr_texture *t);
static bool update_sampler();
static bool update_descriptor();



void gr_texture_create(graphics *gr)
{

	gfx = gr;

}



void gr_texture_destroy()
{

	while (!list_empty(&textures))
		gr_texture_del(list_front(&textures));

	gfx = NULL;

}



gr_texture *gr_texture_new(const char *name)
{

	gr_texture *t = malloc(sizeof(gr_texture) + strlen(name) + 1);

	strcpy((char*)t->name, name);
	reset(t);

	list_init(  &t->node, t);
	list_append(&textures, &t->node);

	return t;

}



gr_texture *gr_texture_open(const char *file)
{

	return gr_texture_openfd(blob_open(file, BLOB_REV_LAST));

}



gr_texture *gr_texture_openfd(int fd)
{

	log_i("texture: loading %s", blob_get_name(fd));

	gr_dds      dds;
	gr_texture *t = gr_texture_new(blob_get_name(fd));

	if (!gr_dds_openfd(&dds, fd)) {

		log_e("texture: failed");
		goto failed;

	}

	if (!gr_texture_alloc(t, dds.format,
			dds.image.width, dds.image.height, dds.image.depth,
			dds.mipmap_count, dds.is_cubemap? 6 * dds.image_count: dds.image_count,
			dds.is_cubemap? GR_TEXTURE_CUBEMAP: GR_TEXTURE_DEFAULT))
		goto failed;

	if (!gr_texture_set_environment(t))
		goto failed;

	{
		size_t count = t->layers * t->mipmaps;

		uint offset[count];
		uint mipmap[count];
		uint layer[count];

		for (int n=0, k=0; n < t->layers; n++)
			for (int m=0; m < t->mipmaps; m++, k++) {

				gr_dds_select(&dds, n, m);

				offset[k] = dds.mipmap.offset;
				mipmap[k] = m;
				layer[k]  = n;

			}

		gr_dds_select(&dds, 0, 0);

		if (!gr_texture_copy(t,
				dds.image.pixels, dds.image.length * t->layers,
				offset, mipmap, layer,
				count))
			goto failed;

	}

	return t;

failed:
	gr_texture_del(t);
	return NULL;


}



void gr_texture_del(gr_texture *t)
{

	if (t == NULL)
		return;

	list_remove(&t->node);

	destroy(t);
	free(t);

}



bool gr_texture_alloc(gr_texture *t,
		const gr_pixelformat *pf,
		uint width,   uint height, uint depth,
		uint mipmaps, uint layers,
		uint flags)
{

	destroy(t);

	t->width   = width;
	t->height  = height;
	t->depth   = depth;
	t->pf      = pf;
	t->flags   = flags;
	t->mipmaps = mipmaps;
	t->layers  = layers;

	uint cflags = 0;

	cflags |= (flags & GR_TEXTURE_CUBEMAP)? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT: 0;

	if (!gr_create_image(&t->image, &t->image_memory,
			pf->format,
			t->width, t->height, t->depth,
			t->mipmaps, t->layers,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, flags)) {

		destroy(t);
		return false;

	}

	if (!gr_create_image_view(&t->image_view, t->image, t->pf->format)) {

		destroy(t);
		return false;

	}

	return true;

}



bool gr_texture_copy(gr_texture *t, const void *pixels, size_t len, uint offset[], uint mipmap[], uint layer[], uint count)
{

	VkBuffer       stage_buf = NULL;
	VkDeviceMemory stage_mem = NULL;

	gr_create_buffer(&stage_buf, &stage_mem,
			len,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (stage_buf == NULL)
		return false;

	gr_upload(pixels, stage_mem, len);

	gr_transition_layout(t->image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);

	gr_mcopy_buffer_to_image(stage_buf, t->image,
			t->width, t->height, t->depth,
			offset, mipmap, layer,
			count);

	gr_transition_layout(t->image,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	vkDestroyBuffer(gfx->vk.gpu, stage_buf, NULL);
	vkFreeMemory(   gfx->vk.gpu, stage_mem, NULL);

	return true;

}



bool gr_texture_set_environment(gr_texture *t)
{

	VkSampler sampler = t->sampler;

	t->sampler = NULL;

	if (!update_sampler() || !update_descriptor(t)) {

		if (t->sampler != NULL)
			vkDestroySampler(gfx->vk.gpu, t->sampler, NULL);

		t->sampler = sampler;
		return false;

	}

	if (sampler != NULL)
		vkDestroySampler(gfx->vk.gpu, sampler, NULL);

	return true;

}



gr_texture *gr_texture_find(const char *name)
{

	for (list *it=list_begin(&textures); it != list_end(&textures); it = it->next)
		if (!strcmp(LIST_PTR(gr_texture, it)->name, name))
			return it->ptr;

	return NULL;

}



bool gr_texture_set_default_filter(int filter)
{

	if (filter < 0 || filter >= GR_TEXTURE_FILTER_MAX)
		return false;

	default_filter = filter;

	for (list *it=list_begin(&textures); it != list_end(&textures); it = it->next) {

		gr_texture *t = LIST_PTR(gr_texture, it);

		if (t->filter == GR_TEXTURE_FILTER_DEFAULT)
			gr_texture_set_environment(t);

	}

	return true;

}



int gr_texture_get_default_filter()
{

	return default_filter;

}



void reset(gr_texture *t)
{

	t->filter     = GR_TEXTURE_FILTER_DEFAULT;
	t->clamp[0]   = GR_TEXCOORD_CLAMP_TO_EDGE;
	t->clamp[1]   = GR_TEXCOORD_CLAMP_TO_EDGE;
	t->clamp[2]   = GR_TEXCOORD_CLAMP_TO_EDGE;
	t->anisotropy = -1.0f;
	t->normalized = true;
	t->comparator = VK_COMPARE_OP_MAX_ENUM;
	t->lod_min    = 0.0f;
	t->lod_max    = 0.0f;
	t->lod_bias   = 0.0f;

	t->width  = 0;
	t->height = 0;
	t->depth  = 0;

	t->pf = gr_pixelformat_get(VK_FORMAT_UNDEFINED);

	t->flags   = GR_TEXTURE_DEFAULT;
	t->mipmaps = 0;
	t->layers  = 0;

	t->descriptor = NULL;
	t->sampler    = NULL;

	t->image        = NULL;
	t->image_memory = NULL;
	t->image_view   = NULL;

}



void destroy(gr_texture *t)
{

	if (t->sampler != NULL)
		vkDestroySampler(gfx->vk.gpu, t->sampler, NULL);

	if (t->image_view != NULL)
		vkDestroyImageView(gfx->vk.gpu, t->image_view, NULL);

	if (t->image != NULL)
		vkDestroyImage(gfx->vk.gpu, t->image, NULL);

	if (t->image_memory != NULL)
		vkFreeMemory(gfx->vk.gpu, t->image_memory, NULL);

	reset(t);

}



bool update_sampler(gr_texture *t)
{

	VkSamplerCreateInfo sci = { 0 };

	const struct mipmap_filter *tf = &mipmap_filter_type[(t->filter == GR_TEXTURE_FILTER_DEFAULT)? default_filter: t->filter];

	sci.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sci.magFilter               = tf->magnification;
	sci.minFilter               = tf->minification;
	sci.addressModeU            = CLAMP_MODE(t->clamp[0]);
	sci.addressModeV            = CLAMP_MODE(t->clamp[1]);
	sci.addressModeW            = CLAMP_MODE(t->clamp[2]);
	sci.anisotropyEnable        = (t->anisotropy > 0.0f)? VK_TRUE: VK_FALSE;
	sci.maxAnisotropy           = (t->anisotropy > 0.0f)? t->anisotropy: 0.0f;
	sci.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sci.unnormalizedCoordinates = t->normalized? VK_FALSE: VK_TRUE;
	sci.compareEnable           = (t->comparator != VK_COMPARE_OP_MAX_ENUM)? VK_TRUE: VK_FALSE;
	sci.compareOp               = (t->comparator != VK_COMPARE_OP_MAX_ENUM)? t->comparator: VK_COMPARE_OP_ALWAYS;
	sci.mipmapMode              = tf->mipmapping;
	sci.mipLodBias              = t->lod_bias;
	sci.minLod                  = t->lod_min;
	sci.maxLod                  = t->lod_max;

	if (vkCreateSampler(gfx->vk.gpu, &sci, NULL, &t->sampler) != VK_SUCCESS)
		return false;

	return true;

}



bool update_descriptor(gr_texture *t)
{

	if (t->descriptor == NULL) {

		VkDescriptorSetLayout       dsl[] = { gfx->vk.descriptor_texture_layout };
		VkDescriptorSetAllocateInfo dsai  = { 0 };

		dsai.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		dsai.descriptorPool     = gfx->vk.descriptor_texture_pool;
		dsai.descriptorSetCount = 1;
		dsai.pSetLayouts        = dsl;

		if (vkAllocateDescriptorSets(gfx->vk.gpu, &dsai, &t->descriptor) != VK_SUCCESS)
			return false;

	}

	VkDescriptorImageInfo dii = { 0 };
	VkWriteDescriptorSet  wds = { 0 };

	dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	dii.imageView   = t->image_view;
	dii.sampler     = t->sampler;

	wds.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds.dstSet           = t->descriptor;
	wds.dstBinding       = 0;
	wds.dstArrayElement  = 0;
	wds.descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	wds.descriptorCount  = 1;
	wds.pBufferInfo      = NULL;
	wds.pImageInfo       = &dii;
	wds.pTexelBufferView = NULL;

	vkUpdateDescriptorSets(gfx->vk.gpu, 1, &wds, 0, NULL);
	return true;

}

