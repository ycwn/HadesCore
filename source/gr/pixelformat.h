

#ifndef __GR_PIXELFORMAT_H
#define __GR_PIXELFORMAT_H


enum {
	GR_PIXELFORMAT_PACKING_LINEAR,
	GR_PIXELFORMAT_PACKING_BLOCK,
	GR_PIXELFORMAT_PACKING_PACKED
};


typedef struct _gr_pixelformat_t {

	const char *name;

	int format;
	int channels;

	int bits;
	int bits_r;
	int bits_g;
	int bits_b;
	int bits_a;
	int bits_d;
	int bits_s;

	int packing;
	int alignment;

	VkFormatProperties properties;

} gr_pixelformat;


void gr_pixelformat_create(graphics *gr);
void gr_pixelformat_destroy();

bool gr_pixelformat_init();

const gr_pixelformat *gr_pixelformat_get(int format);


#endif

