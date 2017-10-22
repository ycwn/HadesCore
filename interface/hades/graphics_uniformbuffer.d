

import hades.graphics_types;
import hades.graphics_graphics;


struct gr_uniformbuffer {

	size_t capacity;

	VkBuffer        buffer;
	VkDeviceMemory  memory;
	VkDescriptorSet descriptor;

}


extern(C) {

	void gr_uniformbuffer_create(graphics *gr);
	void gr_uniformbuffer_destroy();

	void gr_uniformbuffer_init(gr_uniformbuffer *vb);
	void gr_uniformbuffer_del( gr_uniformbuffer *vb);

	bool gr_uniformbuffer_commit(gr_uniformbuffer *vb, const void *buf, size_t len);

}

