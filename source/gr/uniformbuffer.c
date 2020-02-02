

#include "core/system.h"
#include "core/types.h"
#include "core/common.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/blob.h"
#include "core/logger.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/uniformbuffer.h"


static graphics *gfx = NULL;



void gr_uniformbuffer_create(graphics *gr)
{

	gfx = gr;

}



void gr_uniformbuffer_destroy()
{

	gfx = NULL;

}



void gr_uniformbuffer_init(gr_uniformbuffer *ub)
{

	ub->capacity   = 0;
	ub->buffer     = NULL;
	ub->memory     = NULL;
	ub->descriptor = NULL;

}



void gr_uniformbuffer_del(gr_uniformbuffer *ub)
{

	if (ub->buffer != NULL)
		vkDestroyBuffer(gfx->vk.gpu, ub->buffer, NULL);

	if (ub->memory != NULL)
		vkFreeMemory(gfx->vk.gpu, ub->memory, NULL);

	gr_uniformbuffer_init(ub);

}



bool gr_uniformbuffer_commit(gr_uniformbuffer *ub, const void *buf, size_t len)
{

	if (buf == NULL || len > ub->capacity)
		gr_uniformbuffer_del(ub);

	if (buf == NULL)
		return true;

	if (ub->descriptor == NULL) { //FIXME: Descriptor not released? [[PROPER UNIFORM BUFFER SUPPORT]]

		VkDescriptorSetLayout       dsl[] = { gfx->vk.descriptor_uniform_layout };
		VkDescriptorSetAllocateInfo dsai  = { 0 };

		dsai.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		dsai.descriptorPool     = gfx->vk.descriptor_uniform_pool;
		dsai.descriptorSetCount = 1;
		dsai.pSetLayouts        = dsl;

		if (vkAllocateDescriptorSets(gfx->vk.gpu, &dsai, &ub->descriptor) != VK_SUCCESS)
			return false;

	}

	if (ub->buffer == NULL) {

		if (ub->capacity < len)
			ub->capacity = len;

		if (!gr_create_buffer(
				&ub->buffer,  &ub->memory,
				ub->capacity, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
			return false;

	}

	gr_upload(buf, ub->memory, len);

	VkDescriptorBufferInfo dbi = { 0 };
	VkWriteDescriptorSet   wds = { 0 };

	dbi.buffer = ub->buffer;
	dbi.offset = 0;
	dbi.range  = len;

	wds.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds.dstSet           = ub->descriptor;
	wds.dstBinding       = 0;
	wds.dstArrayElement  = 0;
	wds.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	wds.descriptorCount  = 1;
	wds.pBufferInfo      = &dbi;
	wds.pImageInfo       = NULL;
	wds.pTexelBufferView = NULL;

	vkUpdateDescriptorSets(gfx->vk.gpu, 1, &wds, 0, NULL);

	return true;

}

