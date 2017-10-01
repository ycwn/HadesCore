

#ifndef __GR_TEXTURE_H
#define __GR_TEXTURE_H


enum {
	GR_TEXTURE_FILTER_DEFAULT = -1,
	GR_TEXTURE_FILTER_NEAREST,
	GR_TEXTURE_FILTER_LINEAR,
	GR_TEXTURE_FILTER_BILINEAR,
	GR_TEXTURE_FILTER_TRILINEAR,
	GR_TEXTURE_FILTER_MAX
};

enum {
	GR_TEXCOORD_REPEAT,
	GR_TEXCOORD_MIRRORED_REPEAT,
	GR_TEXCOORD_CLAMP_TO_EDGE,
	GR_TEXCOORD_CLAMP_TO_BORDER,
	GR_TEXCOORD_MIRROR_CLAMP_TO_EDGE
};

enum {
	GR_TEXTURE_DEFAULT = 0,
	GR_TEXTURE_CUBEMAP = 1 << 0
};


typedef struct gr_texture {

	int   filter;
	int   clamp[3];
	float anisotropy;
	bool  normalized;
	int   comparator;
	float lod_min;
	float lod_max;
	float lod_bias;

	uint width;
	uint height;
	uint depth;

	const gr_pixelformat *pf;

	uint flags;
	uint mipmaps;
	uint layers;

	VkDescriptorSet descriptor;
	VkSampler       sampler;
	VkImage         image;
	VkDeviceMemory  image_memory;
	VkImageView     image_view;

	list       node;
	const char name[];

} gr_texture;



void gr_texture_create(graphics *gr);
void gr_texture_destroy();

gr_texture *gr_texture_new( const char *name);
gr_texture *gr_texture_open(const char *file);
gr_texture *gr_texture_openfd(int fd);
void        gr_texture_del(gr_texture *t);

bool gr_texture_alloc(gr_texture *t, const gr_pixelformat *pf, uint width, uint height, uint depth, uint mipmaps, uint layers, uint flags);
bool gr_texture_copy( gr_texture *t, const void *pixels, size_t len, uint offset[], uint mipmap[], uint layer[], uint count);
bool gr_texture_set_environment(gr_texture *t);

gr_texture *gr_texture_find(const char *name);

bool gr_texture_set_default_filter(int filter);
int  gr_texture_get_default_filter();


static inline bool gr_texture_is_cube(const gr_texture *t) { return (t->flags & GR_TEXTURE_CUBEMAP) != 0; }
static inline bool gr_texture_is_1d(  const gr_texture *t) { return !gr_texture_is_cube(t) && (t->height == 1) && (t->depth == 1); }
static inline bool gr_texture_is_2d(  const gr_texture *t) { return !gr_texture_is_cube(t) && (t->height != 1) && (t->depth == 1); }
static inline bool gr_texture_is_3d(  const gr_texture *t) { return !gr_texture_is_cube(t) && (t->depth  != 1); }


#endif

