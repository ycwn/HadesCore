

import hades.core_list;

import hades.graphics_types;
import hades.graphics_graphics;
import hades.graphics_pixelformat;


enum {
	GR_SURFACE_FILTER_DEFAULT = -1,
	GR_SURFACE_FILTER_NEAREST,
	GR_SURFACE_FILTER_LINEAR,
	GR_SURFACE_FILTER_BILINEAR,
	GR_SURFACE_FILTER_TRILINEAR,
	GR_SURFACE_FILTER_MAX
}

enum {
	GR_TEXCOORD_REPEAT,
	GR_TEXCOORD_MIRRORED_REPEAT,
	GR_TEXCOORD_CLAMP_TO_EDGE,
	GR_TEXCOORD_CLAMP_TO_BORDER,
	GR_TEXCOORD_MIRROR_CLAMP_TO_EDGE
}

enum {
	GR_SURFACE_DEFAULT    = 0,
	GR_SURFACE_ATTACHMENT = 1 << 0,
	GR_SURFACE_CUBEMAP    = 1 << 1
}


struct gr_surface {

	int    filter;
	int[3] clamp;
	float  anisotropy;
	bool   normalized;
	int    comparator;
	float  lod_min;
	float  lod_max;
	float  lod_bias;

	int id;
	int index;

	uint width;
	uint height;
	uint depth;

	const gr_pixelformat *pf;

	uint flags;
	uint mipmaps;
	uint layers;

	VkSampler       sampler;
	VkImage         image;
	VkDeviceMemory  image_memory;
	VkImageView     image_view;

	list           node;
	const(char)[0] name;

}


extern(C) {

	void            gr_surface_create(graphics *gr);
	void            gr_surface_destroy();
	void            gr_surface_update_cache();
	VkDescriptorSet gr_surface_get_descriptor();

	gr_surface *gr_surface_new( const char *name);
	gr_surface *gr_surface_open(const char *file);
	gr_surface *gr_surface_openfd(int fd);
	gr_surface *gr_surface_attachment(const char *name, uint width, uint height, uint format, bool sampled);
	void        gr_surface_del(gr_surface *s);

	bool gr_surface_alloc(gr_surface *s, const gr_pixelformat *pf, uint width, uint height, uint depth, uint mipmaps, uint layers, uint flags);
	bool gr_surface_copy( gr_surface *s, const void *pixels, size_t len, uint *offset, uint *mipmap, uint *layer, uint count);

	bool gr_surface_bind(gr_surface *s);
	void gr_surface_rebind(gr_surface *s);
	void gr_surface_unbind(gr_surface *s);
	bool gr_surface_set_environment(gr_surface *s);

	gr_surface *gr_surface_find(const char *name);

	bool gr_surface_set_default_filter(int filter);
	int  gr_surface_get_default_filter();

}


bool gr_surface_is_attachment(const gr_surface *s) { return (s.flags & GR_SURFACE_ATTACHMENT) != 0; }
bool gr_surface_is_cube(const gr_surface *s) { return (s.flags & GR_TEXTURE_CUBEMAP) != 0; }
bool gr_surface_is_1d(  const gr_surface *s) { return !gr_surface_is_cube(s) && (s.height == 1) && (s.depth == 1); }
bool gr_surface_is_2d(  const gr_surface *s) { return !gr_surface_is_cube(s) && (s.height != 1) && (s.depth == 1); }
bool gr_surface_is_3d(  const gr_surface *s) { return !gr_surface_is_cube(s) && (s.depth  != 1); }

