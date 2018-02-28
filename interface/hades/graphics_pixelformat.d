

import hades.graphics_types;
import hades.graphics_graphics;


enum {
	GR_PIXELFORMAT_PACKING_LINEAR,
	GR_PIXELFORMAT_PACKING_BLOCK,
	GR_PIXELFORMAT_PACKING_PACKED
}


struct gr_pixelformat {

	const(char) *name;

	int format;
	int channels;

	int bits;
	int bits_r, bits_g, bits_b, bits_a;
	int bits_d, bits_s;

	int packing;

	int block_w;
	int block_h;
	int block_d;

	int alignment;

	VkFormatProperties properties;

}


extern(C) {

	void gr_pixelformat_create(graphics *gr);
	void gr_pixelformat_destroy();

	bool gr_pixelformat_init();

	const(gr_pixelformat) *gr_pixelformat_get(int format);
	const(gr_pixelformat) *gr_pixelformat_find(const(char) *name);

}

