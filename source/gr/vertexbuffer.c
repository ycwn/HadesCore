

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
#include "gr/vertexbuffer.h"


static graphics *gfx = NULL;

static void reset  (struct gr_vbuffer *buf);
static void destroy(struct gr_vbuffer *buf);
static bool commit( struct gr_vbuffer *dst, int usage, const void *src, size_t len);



void gr_vertexbuffer_create(graphics *gr)
{

	gfx = gr;

}



void gr_vertexbuffer_destroy()
{

	gfx = NULL;

}



void gr_vertexbuffer_init(gr_vertexbuffer *vb)
{

	reset(&vb->vertex);
	reset(&vb->index);

}



void gr_vertexbuffer_del(gr_vertexbuffer *vb)
{

	destroy(&vb->vertex);
	destroy(&vb->index);

}



bool gr_vertexbuffer_commit_vertices(gr_vertexbuffer *vb, const void *buf, size_t len)
{

	return commit(&vb->vertex, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, buf, len);

}



bool gr_vertexbuffer_commit_indices( gr_vertexbuffer *vb, const void *buf, size_t len)
{

	return commit(&vb->index, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, buf, len);

}



void reset(struct gr_vbuffer *buf)
{

	buf->capacity = 0;
	buf->buffer   = NULL;
	buf->memory   = NULL;

}



void destroy(struct gr_vbuffer *buf)
{

	if (buf->buffer != NULL)
		vkDestroyBuffer(gfx->vk.gpu, buf->buffer, NULL);

	if (buf->memory != NULL)
		vkFreeMemory(gfx->vk.gpu, buf->memory, NULL);

	reset(buf);

}



bool commit(struct gr_vbuffer *dst, int usage, const void *buf, size_t len)
{

	if (buf == NULL || len > dst->capacity)
		destroy(dst);

	if (buf == NULL)
		return true;

	if (dst->buffer == NULL) {

		if (dst->capacity < len)
			dst->capacity = len;

		if (!gr_create_buffer(
				&dst->buffer,  &dst->memory,
				dst->capacity, usage,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
			return false;

	}

	gr_upload(buf, dst->memory, len);

	return true;

}

