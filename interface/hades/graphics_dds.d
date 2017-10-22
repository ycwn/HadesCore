

import hades.graphics_pixelformat;


struct gr_dds {

	const gr_pixelformat *format;
	const ubyte          *data;

	uint image_count;
	uint mipmap_count;
	bool is_cubemap;

	struct descriptor {

		const ubyte *pixels;

		uint width;
		uint height;
		uint depth;
		uint length;
		uint offset;

	}

	descriptor image, mipmap;

}


extern(C) {

	void gr_dds_init(  gr_dds *dds);
	bool gr_dds_open(  gr_dds *dds, const char *file);
	bool gr_dds_openfd(gr_dds *dds, int id);
	bool gr_dds_load(  gr_dds *dds, const void *ptr);
	bool gr_dds_select(gr_dds *dds, uint image, uint mipmap);

}

bool gr_dds_is_valid(const gr_dds *dds) { return dds.format != VK_FORMAT_UNDEFINED; }
bool gr_dds_is_1d(   const gr_dds *dds) { return dds.image.depth <= 1 && dds.image.height <= 1; }
bool gr_dds_is_2d(   const gr_dds *dds) { return dds.image.depth <= 1 && dds.image.height > 1;  }
bool gr_dds_is_3d(   const gr_dds *dds) { return dds.image.depth > 1; }
bool gr_dds_is_array(const gr_dds *dds) { return dds.image_count > 1; }

