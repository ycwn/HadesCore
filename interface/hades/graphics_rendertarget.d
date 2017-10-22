

import hades.core_list;

import hades.graphics_limits;
import hades.graphics_types;
import hades.graphics_graphics;


enum GR_RENDERPASS_DEFAULT = 0x0000;

enum GR_RENDERPASS_LOAD_DONTCARE = 0x0000;
enum GR_RENDERPASS_LOAD_CLEAR    = 0x0001;
enum GR_RENDERPASS_LOAD_PRESERVE = 0x0003;

enum GR_RENDERPASS_STORE_DONTCARE = 0x0000;
enum GR_RENDERPASS_STORE_PRESERVE = 0x0004;

enum GR_RENDERPASS_LAYOUT_IN_UNDEFINED       = 0x0000;
enum GR_RENDERPASS_LAYOUT_IN_GENERAL         = 0x0010;
enum GR_RENDERPASS_LAYOUT_IN_C_OPTIMAL       = 0x0020;
enum GR_RENDERPASS_LAYOUT_IN_DS_OPTIMAL      = 0x0030;
enum GR_RENDERPASS_LAYOUT_IN_DS_READONLY     = 0x0040;
enum GR_RENDERPASS_LAYOUT_IN_SHADER_READONLY = 0x0060;
enum GR_RENDERPASS_LAYOUT_IN_PREINIT         = 0x0080;

enum GR_RENDERPASS_LAYOUT_OUT_GENERAL    = 0x0000;
enum GR_RENDERPASS_LAYOUT_OUT_C_OPTIMAL  = 0x0100;
enum GR_RENDERPASS_LAYOUT_OUT_DS_OPTIMAL = 0x0200;
enum GR_RENDERPASS_LAYOUT_OUT_PRESENT    = 0x0300;


struct gr_rendertarget {

	VkRenderPass  renderpass;
	VkFramebuffer framebuffer;

	int width;
	int height;

	int attachment_count;
	//int attachment_colors;

	VkImageView[GR_ATTACHMENTS_MAX]   attachment_image;
	VkClearValue[GR_ATTACHMENTS_MAX]  attachment_clear;
	int[GR_ATTACHMENTS_MAX]           attachment_format;
	uint[GR_ATTACHMENTS_MAX]          attachment_flags;

	list           node;
	const(char)[0] name;

}


extern(C) {

	void gr_rendertarget_create(graphics *gr);
	void gr_rendertarget_destroy();

	gr_rendertarget *gr_rendertarget_new(const char *name);
	void             gr_rendertarget_del(gr_rendertarget *rt);

	gr_rendertarget *gr_rendertarget_find(const char *name);

	int  gr_rendertarget_append(gr_rendertarget *rt, VkImageView image, const VkClearValue *clear, int format, uint flags);
	bool gr_rendertarget_build( gr_rendertarget *rt);
	bool gr_rendertarget_build_multi(gr_rendertarget *rt, int slot, const VkImageView *images, VkFramebuffer *buffers, int num);

	VkRenderPass  gr_rendertarget_create_renderpass( const gr_rendertarget *rt); //FIXME: Remove
	VkFramebuffer gr_rendertarget_create_framebuffer(const gr_rendertarget *rt); //FIXME: Remove

}

