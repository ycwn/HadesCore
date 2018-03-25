

import hades.core_list;

import hades.graphics_limits;
import hades.graphics_types;
import hades.graphics_graphics;
import hades.graphics_surface;

import gpuio;


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

enum GR_RENDERPASS_LAYOUT_OUT_GENERAL     = 0x0000;
enum GR_RENDERPASS_LAYOUT_OUT_C_OPTIMAL   = 0x0100;
enum GR_RENDERPASS_LAYOUT_OUT_DS_OPTIMAL  = 0x0200;
enum GR_RENDERPASS_LAYOUT_OUT_PRESENT     = 0x0300;
enum GR_RENDERPASS_LAYOUT_OUT_SHADER_READ = 0x0400;


struct gr_renderpass {

	VkRenderPass                     renderpass;
	VkFramebuffer[GR_SWAPCHAIN_MAX]  framebuffer;

	int width;
	int height;

	int attachment_colors;
	int attachment_count;
	int surface_count;

	VkImageView[GR_SWAPCHAIN_MAX][GR_ATTACHMENTS_MAX]  attachment_image;
	VkClearValue[GR_ATTACHMENTS_MAX]                   attachment_clear;
	int[GR_ATTACHMENTS_MAX]                            attachment_format;
	uint[GR_ATTACHMENTS_MAX]                           attachment_flags;
	uint[GPU_SHADER_TEXTURES]                          surface_ids;

	list           node;
	const(char)[0] name;

}


extern(C) {

	void gr_renderpass_create(graphics *gr);
	void gr_renderpass_destroy();

	gr_renderpass *gr_renderpass_new(const(char) *name);
	void           gr_renderpass_del(gr_renderpass *rp);

	gr_renderpass *gr_renderpass_find(const(char) *name);

	int  gr_renderpass_attach( gr_renderpass *rp, VkImageView image, const(VkClearValue) *clear, int format, uint flags);
	int  gr_renderpass_surface(gr_renderpass *rp, gr_surface *surf);
	bool gr_renderpass_build(  gr_renderpass *rp);
	bool gr_renderpass_bind(   gr_renderpass *rp);

}

