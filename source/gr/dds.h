

#ifndef __GL_DDS_H
#define __GL_DDS_H


typedef struct _gr_dds_t {

	const gr_pixelformat *format;
	const u8             *data;

	uint image_count;
	uint mipmap_count;
	bool is_cubemap;

	struct {

		const u8 *pixels;

		uint width;
		uint height;
		uint depth;
		uint length;
		uint offset;

	} image, mipmap;

} gr_dds;


void gr_dds_init(  gr_dds *dds);
bool gr_dds_open(  gr_dds *dds, const char *file);
bool gr_dds_openfd(gr_dds *dds, int id);
bool gr_dds_load(  gr_dds *dds, const void *ptr);
bool gr_dds_select(gr_dds *dds, uint image, uint mipmap);

static inline bool gr_dds_is_valid(const gr_dds *dds) { return dds->format != VK_FORMAT_UNDEFINED; }
static inline bool gr_dds_is_1d(   const gr_dds *dds) { return dds->image.depth <= 1 && dds->image.height <= 1; }
static inline bool gr_dds_is_2d(   const gr_dds *dds) { return dds->image.depth <= 1 && dds->image.height > 1;  }
static inline bool gr_dds_is_3d(   const gr_dds *dds) { return dds->image.depth > 1; }
static inline bool gr_dds_is_array(const gr_dds *dds) { return dds->image_count > 1; }


#endif

