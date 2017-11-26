

#ifndef GR_VERTEXBUFFER_H
#define GR_VERTEXBUFFER_H


struct gr_vbuffer {

	size_t capacity;

	VkBuffer       buffer;
	VkDeviceMemory memory;

};

typedef struct gr_vertexbuffer {

	struct gr_vbuffer vertex;
	struct gr_vbuffer index;

} gr_vertexbuffer;


void gr_vertexbuffer_create(graphics *gr);
void gr_vertexbuffer_destroy();

void gr_vertexbuffer_init(gr_vertexbuffer *vb);
void gr_vertexbuffer_del( gr_vertexbuffer *vb);

bool gr_vertexbuffer_commit_vertices(gr_vertexbuffer *vb, const void *buf, size_t len);
bool gr_vertexbuffer_commit_indices( gr_vertexbuffer *vb, const void *buf, size_t len);


#endif

