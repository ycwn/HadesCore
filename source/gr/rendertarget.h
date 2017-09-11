

#ifndef __GR_RENDERTARGET_H
#define __GR_RENDERTARGET_H


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

#define GR_RENDERPASS_LAYOUT_OUT_GENERAL     0x0000
#define GR_RENDERPASS_LAYOUT_OUT_C_OPTIMAL   0x0100
#define GR_RENDERPASS_LAYOUT_OUT_DS_OPTIMAL  0x0200
#define GR_RENDERPASS_LAYOUT_OUT_PRESENT     0x0300


typedef struct _gr_rendertarget_t {

	VkRenderPass  renderpass;
	VkFramebuffer framebuffer;

	int width;
	int height;

	int attachment_count;
	//int attachment_colors;

	VkImageView   attachment_image[GR_ATTACHMENTS_MAX];
	VkClearValue  attachment_clear[GR_ATTACHMENTS_MAX];
	int           attachment_format[GR_ATTACHMENTS_MAX];
	uint          attachment_flags[GR_ATTACHMENTS_MAX];

	list node;
	char name[];

} gr_rendertarget;


void gr_rendertarget_create(graphics *gr);
void gr_rendertarget_destroy();

gr_rendertarget *gr_rendertarget_new(const char *name);
void             gr_rendertarget_del(gr_rendertarget *rt);

gr_rendertarget *gr_rendertarget_find(const char *name);

int  gr_rendertarget_append(gr_rendertarget *rt, VkImageView image, const VkClearValue *clear, int format, uint flags);
bool gr_rendertarget_build( gr_rendertarget *rt);
bool gr_rendertarget_build_multi(gr_rendertarget *rt, int slot, const VkImageView images[], VkFramebuffer buffers[], int num);

VkRenderPass  gr_rendertarget_create_renderpass( const gr_rendertarget *rt); //FIXME: Remove
VkFramebuffer gr_rendertarget_create_framebuffer(const gr_rendertarget *rt); //FIXME: Remove


#endif

