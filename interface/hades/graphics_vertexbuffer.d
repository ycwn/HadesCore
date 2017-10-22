

import hades.graphics_types;
import hades.graphics_graphics;


struct gr_vertexbuffer {

	struct vbuffer {

		size_t capacity;

		VkBuffer       buffer;
		VkDeviceMemory memory;

	}

	vbuffer vertex;
	vbuffer index;

}


extern(C) {

	void gr_vertexbuffer_create(graphics *gr);
	void gr_vertexbuffer_destroy();

	void gr_vertexbuffer_init(gr_vertexbuffer *vb);
	void gr_vertexbuffer_del( gr_vertexbuffer *vb);

	bool gr_vertexbuffer_commit_vertices(gr_vertexbuffer *vb, const void *buf, size_t len);
	bool gr_vertexbuffer_commit_indices( gr_vertexbuffer *vb, const void *buf, size_t len);

}

