

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/blob.h"
#include "core/logger.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/pixelformat.h"
#include "gr/surface.h"
#include "gr/dds.h"


#define CLAMP_MODE(x) (                                                                     \
	((x) == GR_TEXCOORD_REPEAT)?               VK_SAMPLER_ADDRESS_MODE_REPEAT:          \
	((x) == GR_TEXCOORD_MIRRORED_REPEAT)?      VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: \
	((x) == GR_TEXCOORD_CLAMP_TO_EDGE)?        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:   \
	((x) == GR_TEXCOORD_CLAMP_TO_BORDER)?      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER: \
	((x) == GR_TEXCOORD_MIRROR_CLAMP_TO_EDGE)? VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE: VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)


struct texture_filter {

	int minification;
	int magnification;
	int mipmapping;

};


static list      textures       = LIST_INIT(&textures, NULL);
static int       default_filter = GR_SURFACE_FILTER_BILINEAR;
static graphics *gfx            = NULL;

static u64                   texture_cache_bitmap[(GPU_TEXTURES_NUM + 63) / 64];
static VkDescriptorImageInfo texture_cache_imageinfo[GPU_TEXTURES_NUM];
static VkDescriptorSet       texture_cache_descriptor = NULL;
static bool                  texture_cache_update     = false;

static VkImage         texture_guard_image        = NULL;
static VkDeviceMemory  texture_guard_image_memory = NULL;
static VkImageView     texture_guard_image_view   = NULL;
static VkSampler       texture_guard_sampler      = NULL;


static const struct texture_filter mipmap_filter_type[] = {

	{ VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST }, // No filtering
	{ VK_FILTER_LINEAR,  VK_FILTER_LINEAR,  VK_SAMPLER_MIPMAP_MODE_NEAREST }, // Linear
	{ VK_FILTER_LINEAR,  VK_FILTER_LINEAR,  VK_SAMPLER_MIPMAP_MODE_NEAREST }, // Bilinear
	{ VK_FILTER_LINEAR,  VK_FILTER_LINEAR,  VK_SAMPLER_MIPMAP_MODE_LINEAR  }, // Trilinear

};


static void reset(  gr_surface *t);
static void destroy(gr_surface *t);



void gr_surface_create(graphics *gr)
{

	gfx = gr;

	for (int n=0; n < GPU_TEXTURES_NUM; n++) {

		texture_cache_imageinfo[n].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		texture_cache_imageinfo[n].imageView   = NULL;
		texture_cache_imageinfo[n].sampler     = NULL;

	}

	texture_cache_update = true;

}



void gr_surface_destroy()
{

	while (!list_empty(&textures))
		gr_surface_del(list_front(&textures));


	if (texture_guard_sampler != NULL)
		vkDestroySampler(gfx->vk.gpu, texture_guard_sampler, NULL);

	if (texture_guard_image_view != NULL)
		vkDestroyImageView(gfx->vk.gpu, texture_guard_image_view, NULL);

	if (texture_guard_image != NULL)
		vkDestroyImage(gfx->vk.gpu, texture_guard_image, NULL);

	if (texture_guard_image_memory != NULL)
		vkFreeMemory(gfx->vk.gpu, texture_guard_image_memory, NULL);


	texture_guard_image        = NULL;
	texture_guard_image_memory = NULL;
	texture_guard_image_view   = NULL;
	texture_guard_sampler      = NULL;

	gfx = NULL;

}



void gr_surface_update_cache()
{

	if (!texture_cache_update)
		return;


	if (texture_guard_image_view == NULL) {

		if (!gr_create_image(&texture_guard_image, &texture_guard_image_memory,
				VK_FORMAT_B8G8R8A8_UNORM,
				1, 1, 1,
				1, 1,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, GR_SURFACE_DEFAULT)) {

			return;

		}

		if (!gr_create_image_view(&texture_guard_image_view, texture_guard_image, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT))
			return;

		gr_transition_layout(texture_guard_image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);

		gr_transition_layout(texture_guard_image,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

		for (int n=0; n < GPU_TEXTURES_NUM; n++)
			if (texture_cache_imageinfo[n].imageView == NULL)
				texture_cache_imageinfo[n].imageView = texture_guard_image_view;

	}


	if (texture_guard_sampler == NULL) {

		VkSamplerCreateInfo sci = { 0 };

		sci.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sci.magFilter               = VK_FILTER_NEAREST;
		sci.minFilter               = VK_FILTER_NEAREST;
		sci.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sci.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sci.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sci.anisotropyEnable        = VK_FALSE;
		sci.maxAnisotropy           = 1.0f;
		sci.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		sci.unnormalizedCoordinates = VK_FALSE;
		sci.compareEnable           = VK_FALSE;
		sci.compareOp               = VK_COMPARE_OP_ALWAYS;
		sci.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		sci.mipLodBias              = 0.0f;
		sci.minLod                  = 0.0f;
		sci.maxLod                  = 0.0f;

		if (vkCreateSampler(gfx->vk.gpu, &sci, NULL, &texture_guard_sampler) != VK_SUCCESS)
			return;

		for (int n=0; n < GPU_TEXTURES_NUM; n++)
			if (texture_cache_imageinfo[n].sampler == NULL)
				texture_cache_imageinfo[n].sampler = texture_guard_sampler;

	}


	if (texture_cache_descriptor == NULL) {

		VkDescriptorSetLayout       dsl[] = { gfx->vk.descriptor_texture_layout };
		VkDescriptorSetAllocateInfo dsai  = { 0 };

		dsai.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		dsai.descriptorPool     = gfx->vk.descriptor_texture_pool;
		dsai.descriptorSetCount = 1;
		dsai.pSetLayouts        = dsl;

		if (vkAllocateDescriptorSets(gfx->vk.gpu, &dsai, &texture_cache_descriptor) != VK_SUCCESS)
			return;

	}

	VkWriteDescriptorSet wds[3] = { 0 };

	wds[0].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds[0].dstSet           = texture_cache_descriptor;
	wds[0].dstBinding       = GPU_TEXTURE_2D_BINDING;
	wds[0].dstArrayElement  = 0;
	wds[0].descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	wds[0].descriptorCount  = GPU_TEXTURES_2D;
	wds[0].pBufferInfo      = NULL;
	wds[0].pImageInfo       = &texture_cache_imageinfo[GPU_TEXTURES_2D_BEGIN];
	wds[0].pTexelBufferView = NULL;

	wds[1].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds[1].dstSet           = texture_cache_descriptor;
	wds[1].dstBinding       = GPU_TEXTURE_3D_BINDING;
	wds[1].dstArrayElement  = 0;
	wds[1].descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	wds[1].descriptorCount  = GPU_TEXTURES_3D;
	wds[1].pBufferInfo      = NULL;
	wds[1].pImageInfo       = &texture_cache_imageinfo[GPU_TEXTURES_3D_BEGIN];
	wds[1].pTexelBufferView = NULL;

	wds[2].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds[2].dstSet           = texture_cache_descriptor;
	wds[2].dstBinding       = GPU_TEXTURE_CUBE_BINDING;
	wds[2].dstArrayElement  = 0;
	wds[2].descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	wds[2].descriptorCount  = GPU_TEXTURES_CUBE;
	wds[2].pBufferInfo      = NULL;
	wds[2].pImageInfo       = &texture_cache_imageinfo[GPU_TEXTURES_CUBE_BEGIN];
	wds[2].pTexelBufferView = NULL;

	vkUpdateDescriptorSets(gfx->vk.gpu, countof(wds), &wds[0], 0, NULL);

	texture_cache_update = false;

}



VkDescriptorSet gr_surface_get_descriptor()
{

	return texture_cache_descriptor;

}



gr_surface *gr_surface_new(const char *name)
{

	gr_surface *s = malloc(sizeof(gr_surface) + strlen(name) + 1);

	strcpy((char*)s->name, name);
	reset(s);

	list_init(  &s->node, s);
	list_append(&textures, &s->node);

	return s;

}



gr_surface *gr_surface_open(const char *file)
{

	return gr_surface_openfd(blob_open(file, BLOB_REV_LAST));

}



gr_surface *gr_surface_openfd(int fd)
{

	log_i("texture: loading %s", blob_get_name(fd));

	gr_dds      dds;
	gr_surface *s = gr_surface_new(blob_get_name(fd));

	if (!gr_dds_openfd(&dds, fd)) {

		log_e("texture: failed");
		goto failed;

	}

	if (!gr_surface_alloc(s, dds.format,
			dds.image.width, dds.image.height, dds.image.depth,
			dds.mipmap_count, dds.is_cubemap? 6 * dds.image_count: dds.image_count,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			dds.is_cubemap? GR_SURFACE_CUBEMAP: GR_SURFACE_DEFAULT))
		goto failed;

	if (!gr_surface_set_environment(s))
		goto failed;

	{
		size_t count = s->layers * s->mipmaps;

		uint offset[count];
		uint mipmap[count];
		uint layer[count];

		for (int n=0, k=0; n < s->layers; n++)
			for (int m=0; m < s->mipmaps; m++, k++) {

				gr_dds_select(&dds, n, m);

				offset[k] = dds.mipmap.offset;
				mipmap[k] = m;
				layer[k]  = n;

			}

		gr_dds_select(&dds, 0, 0);

		if (!gr_surface_copy(s,
				dds.image.pixels, dds.image.length * s->layers,
				offset, mipmap, layer,
				count))
			goto failed;

	}

	return s;

failed:
	gr_surface_del(s);

	return NULL;

}



gr_surface *gr_surface_attachment(const char *name, uint width, uint height, uint format, bool sampled)
{

	const gr_pixelformat *pf = gr_pixelformat_get(format);

	if (pf->format == VK_FORMAT_UNDEFINED)
		return NULL;

	gr_surface *s = gr_surface_new(name);

	uint usage = 0;

	if (pf->aspect & VK_IMAGE_ASPECT_COLOR_BIT)   usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (pf->aspect & VK_IMAGE_ASPECT_DEPTH_BIT)   usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	if (pf->aspect & VK_IMAGE_ASPECT_STENCIL_BIT) usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	if (sampled)                                  usage |= VK_IMAGE_USAGE_SAMPLED_BIT;

	if (!gr_surface_alloc(s, pf, width, height, 1, 1, 1, usage, GR_SURFACE_DEFAULT))
		goto failed;

	return s;

failed:
	gr_surface_del(s);

	return NULL;

}



void gr_surface_del(gr_surface *s)
{

	if (s == NULL)
		return;

	gr_surface_unbind(s);

	list_remove(&s->node);

	destroy(s);
	free(s);

}



bool gr_surface_alloc(gr_surface *s,
		const gr_pixelformat *pf,
		uint width,   uint height, uint depth,
		uint mipmaps, uint layers, uint usage,
		uint flags)
{

	destroy(s);

	s->width   = width;
	s->height  = height;
	s->depth   = depth;
	s->pf      = pf;
	s->flags   = flags;
	s->mipmaps = mipmaps;
	s->layers  = layers;

	uint cflags = 0;

	cflags |= (flags & GR_SURFACE_CUBEMAP)? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT: 0;

	if (!gr_create_image(&s->image, &s->image_memory,
			pf->format,
			s->width, s->height, s->depth,
			s->mipmaps, s->layers,
			VK_IMAGE_TILING_OPTIMAL,
			usage,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, cflags)) {

		destroy(s);
		return false;

	}

	if (!gr_create_image_view(&s->image_view, s->image, s->pf->format, s->pf->aspect)) {

		destroy(s);
		return false;

	}

	return true;

}



bool gr_surface_copy(gr_surface *s, const void *pixels, size_t len, uint offset[], uint mipmap[], uint layer[], uint count)
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

	gr_transition_layout(s->image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			s->pf->aspect,
			VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);

	gr_mcopy_buffer_to_image(stage_buf, s->image,
			s->width, s->height, s->depth,
			offset, mipmap, layer,
			count);

	gr_transition_layout(s->image,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			s->pf->aspect,
			VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	vkDestroyBuffer(gfx->vk.gpu, stage_buf, NULL);
	vkFreeMemory(   gfx->vk.gpu, stage_mem, NULL);

	return true;

}



bool gr_surface_bind(gr_surface *s)
{

	if (s->id < 0) {

		uint begin =
			gr_surface_is_1d(s)?   GPU_TEXTURES_2D_BEGIN:
			gr_surface_is_2d(s)?   GPU_TEXTURES_2D_BEGIN:
			gr_surface_is_3d(s)?   GPU_TEXTURES_3D_BEGIN:
			gr_surface_is_cube(s)? GPU_TEXTURES_CUBE_BEGIN: 0;

		uint end =
			gr_surface_is_1d(s)?   GPU_TEXTURES_2D_END:
			gr_surface_is_2d(s)?   GPU_TEXTURES_2D_END:
			gr_surface_is_3d(s)?   GPU_TEXTURES_3D_END:
			gr_surface_is_cube(s)? GPU_TEXTURES_CUBE_END: 0;

		int block = -1;
		u64 mask  = 0;

		for (int n=begin / 64; n <= (end - 1) / 64; n++) {

			int mask_top    = mini(end   - 64 * n + 1, 64);
			int mask_bottom = maxi(begin - 64 * n,     0);

			mask = bitmask(uint64_t, mask_top) ^ bitmask(uint64_t, mask_bottom);

			if ((texture_cache_bitmap[n] & mask) != mask) {

				block = n;
				break;

			}

		}

		if (block < 0)
			return false; // Oh shit

		int entry = ffsl(~texture_cache_bitmap[block] & mask) - 1; // Find the first unset bit in the mask

		s->index = block * 64 + entry;
		s->id    = s->index - begin;

		texture_cache_bitmap[block] |= 1 << entry;

	}

	gr_surface_rebind(s);

	return true;

}



void gr_surface_rebind(gr_surface *s)
{

	if (s->id < 0 || s->sampler == NULL)
		return;

	texture_cache_imageinfo[s->index].imageView = s->image_view;
	texture_cache_imageinfo[s->index].sampler   = s->sampler;

	texture_cache_update = true;

}



void gr_surface_unbind(gr_surface *s)
{

	if (s->id < 0 || s->sampler == NULL)
		return;

	texture_cache_imageinfo[s->index].imageView = texture_guard_image_view;
	texture_cache_imageinfo[s->index].sampler   = texture_guard_sampler;

	texture_cache_bitmap[s->index / 64] &= ~(1 << (s->index & 63));

	s->id    = -1;
	s->index = -1;

	texture_cache_update = true;

}



void gr_surface_lock(gr_surface *s)
{

	s->lock++;

}



void gr_surface_unlock(gr_surface *s)
{

	s->lock--;

}



bool gr_surface_set_environment(gr_surface *s)
{

	VkSamplerCreateInfo sci = { 0 };

	const struct texture_filter *tf = &mipmap_filter_type[(s->filter == GR_SURFACE_FILTER_DEFAULT)? default_filter: s->filter];

	sci.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sci.magFilter               = tf->magnification;
	sci.minFilter               = tf->minification;
	sci.addressModeU            = CLAMP_MODE(s->clamp[0]);
	sci.addressModeV            = CLAMP_MODE(s->clamp[1]);
	sci.addressModeW            = CLAMP_MODE(s->clamp[2]);
	sci.anisotropyEnable        = (s->anisotropy > 0.0f)? VK_TRUE: VK_FALSE;
	sci.maxAnisotropy           = (s->anisotropy > 0.0f)? s->anisotropy: 1.0f;
	sci.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sci.unnormalizedCoordinates = s->normalized? VK_FALSE: VK_TRUE;
	sci.compareEnable           = (s->comparator != VK_COMPARE_OP_MAX_ENUM)? VK_TRUE: VK_FALSE;
	sci.compareOp               = (s->comparator != VK_COMPARE_OP_MAX_ENUM)? s->comparator: VK_COMPARE_OP_ALWAYS;
	sci.mipmapMode              = tf->mipmapping;
	sci.mipLodBias              = s->lod_bias;
	sci.minLod                  = s->lod_min;
	sci.maxLod                  = s->lod_max;

	VkSampler sampler = s->sampler;

	s->sampler = NULL;

	if (vkCreateSampler(gfx->vk.gpu, &sci, NULL, &s->sampler) != VK_SUCCESS) {

		if (s->sampler != NULL)
			vkDestroySampler(gfx->vk.gpu, s->sampler, NULL);

		s->sampler = sampler;
		return false;

	}

	if (sampler != NULL)
		vkDestroySampler(gfx->vk.gpu, sampler, NULL);

	gr_surface_rebind(s);

	return true;

}



gr_surface *gr_surface_find(const char *name)
{

	for (list *it=list_begin(&textures); it != list_end(&textures); it = it->next)
		if (!strcmp(LIST_PTR(gr_surface, it)->name, name))
			return it->ptr;

	return NULL;

}



bool gr_surface_set_default_filter(int filter)
{

	if (filter < 0 || filter >= GR_SURFACE_FILTER_MAX)
		return false;

	default_filter = filter;

	for (list *it=list_begin(&textures); it != list_end(&textures); it = it->next) {

		gr_surface *s = LIST_PTR(gr_surface, it);

		if (s->filter == GR_SURFACE_FILTER_DEFAULT)
			gr_surface_set_environment(s);

	}

	return true;

}



int gr_surface_get_default_filter()
{

	return default_filter;

}



void reset(gr_surface *s)
{

	s->filter     = GR_SURFACE_FILTER_DEFAULT;
	s->clamp[0]   = GR_TEXCOORD_CLAMP_TO_EDGE;
	s->clamp[1]   = GR_TEXCOORD_CLAMP_TO_EDGE;
	s->clamp[2]   = GR_TEXCOORD_CLAMP_TO_EDGE;
	s->anisotropy = -1.0f;
	s->normalized = true;
	s->comparator = VK_COMPARE_OP_MAX_ENUM;
	s->lod_min    = 0.0f;
	s->lod_max    = 0.0f;
	s->lod_bias   = 0.0f;

	s->id    = -1;
	s->index = -1;

	s->width  = 0;
	s->height = 0;
	s->depth  = 0;

	s->pf = gr_pixelformat_get(VK_FORMAT_UNDEFINED);

	s->flags   = GR_SURFACE_DEFAULT;
	s->mipmaps = 0;
	s->layers  = 0;

	s->sampler      = NULL;
	s->image        = NULL;
	s->image_memory = NULL;
	s->image_view   = NULL;

}



void destroy(gr_surface *s)
{

	if (s->sampler != NULL)
		vkDestroySampler(gfx->vk.gpu, s->sampler, NULL);

	if (s->image_view != NULL)
		vkDestroyImageView(gfx->vk.gpu, s->image_view, NULL);

	if (s->image != NULL)
		vkDestroyImage(gfx->vk.gpu, s->image, NULL);

	if (s->image_memory != NULL)
		vkFreeMemory(gfx->vk.gpu, s->image_memory, NULL);

	reset(s);

}

