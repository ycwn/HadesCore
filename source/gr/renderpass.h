

#ifndef GR_RENDERPASS_H
#define GR_RENDERPASS_H


#define GR_RENDERPASS_DEFAULT  0x0000

#define GR_RENDERPASS_LOAD_DONTCARE   0x0000
#define GR_RENDERPASS_LOAD_CLEAR      0x0001
#define GR_RENDERPASS_LOAD_PRESERVE   0x0003

#define GR_RENDERPASS_STORE_DONTCARE  0x0000
#define GR_RENDERPASS_STORE_PRESERVE  0x0004

#define GR_RENDERPASS_LAYOUT_IN_UNDEFINED        0x0000
#define GR_RENDERPASS_LAYOUT_IN_GENERAL          0x0010
#define GR_RENDERPASS_LAYOUT_IN_C_OPTIMAL        0x0020
#define GR_RENDERPASS_LAYOUT_IN_DS_OPTIMAL       0x0030
#define GR_RENDERPASS_LAYOUT_IN_DS_READONLY      0x0040
#define GR_RENDERPASS_LAYOUT_IN_SHADER_READONLY  0x0060
#define GR_RENDERPASS_LAYOUT_IN_PREINIT          0x0080

#define GR_RENDERPASS_LAYOUT_OUT_GENERAL      0x0000
#define GR_RENDERPASS_LAYOUT_OUT_C_OPTIMAL    0x0100
#define GR_RENDERPASS_LAYOUT_OUT_DS_OPTIMAL   0x0200
#define GR_RENDERPASS_LAYOUT_OUT_PRESENT      0x0300
#define GR_RENDERPASS_LAYOUT_OUT_SHADER_READ  0x0400


typedef struct gr_renderpass {

	VkRenderPass  renderpass;
	VkFramebuffer framebuffer[GR_SWAPCHAIN_MAX];

	int width;
	int height;

	int attachment_count;
	int surface_count;
	//int attachment_colors;

	VkImageView   attachment_image[GR_SWAPCHAIN_MAX][GR_ATTACHMENTS_MAX];
	VkClearValue  attachment_clear[GR_ATTACHMENTS_MAX];
	int           attachment_format[GR_ATTACHMENTS_MAX];
	uint          attachment_flags[GR_ATTACHMENTS_MAX];
	uint          surface_ids[GPU_SHADER_TEXTURES];

	list node;
	char name[];

} gr_renderpass;


void gr_renderpass_create(graphics *gr);
void gr_renderpass_destroy();

gr_renderpass *gr_renderpass_new(const char *name);
void           gr_renderpass_del(gr_renderpass *rp);

gr_renderpass *gr_renderpass_find(const char *name);

int  gr_renderpass_attach( gr_renderpass *rp, VkImageView image, const VkClearValue *clear, int format, uint flags);
int  gr_renderpass_surface(gr_renderpass *rp, gr_surface *surf);
bool gr_renderpass_build(  gr_renderpass *rp);
void gr_renderpass_bind(   gr_renderpass *rp, VkCommandBuffer cb);


#endif

