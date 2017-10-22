

import hades.graphics_graphics;


extern(C) {

	void gr_framebuffer_create(graphics *gr);
	void gr_framebuffer_destroy();

	bool gr_framebuffer_init();
	void gr_framebuffer_select();

}

