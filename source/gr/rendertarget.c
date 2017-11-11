

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/logger.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/rendertarget.h"



#define LOAD_OP(x) \
	((((x) & 0x03) == GR_RENDERPASS_LOAD_CLEAR)?    VK_ATTACHMENT_LOAD_OP_CLEAR: \
	 (((x) & 0x03) == GR_RENDERPASS_LOAD_PRESERVE)? VK_ATTACHMENT_LOAD_OP_LOAD: VK_ATTACHMENT_LOAD_OP_DONT_CARE)

#define STORE_OP(x) \
	((((x) & 0x04) == GR_RENDERPASS_STORE_PRESERVE)? VK_ATTACHMENT_STORE_OP_STORE: VK_ATTACHMENT_STORE_OP_DONT_CARE)

#define LAYOUT_IN(x) \
	((((x) & 0x00f0) == GR_RENDERPASS_LAYOUT_IN_GENERAL)?         VK_IMAGE_LAYOUT_GENERAL: \
	 (((x) & 0x00f0) == GR_RENDERPASS_LAYOUT_IN_C_OPTIMAL)?       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: \
	 (((x) & 0x00f0) == GR_RENDERPASS_LAYOUT_IN_DS_OPTIMAL)?      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: \
	 (((x) & 0x00f0) == GR_RENDERPASS_LAYOUT_IN_DS_READONLY)?     VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: \
	 (((x) & 0x00f0) == GR_RENDERPASS_LAYOUT_IN_SHADER_READONLY)? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: \
	 (((x) & 0x00f0) == GR_RENDERPASS_LAYOUT_IN_PREINIT)?         VK_IMAGE_LAYOUT_PREINITIALIZED: VK_IMAGE_LAYOUT_UNDEFINED)

#define LAYOUT_OUT(x) \
	((((x) & 0x0300) == GR_RENDERPASS_LAYOUT_OUT_C_OPTIMAL)?  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: \
	 (((x) & 0x0300) == GR_RENDERPASS_LAYOUT_OUT_DS_OPTIMAL)? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: \
	 (((x) & 0x0300) == GR_RENDERPASS_LAYOUT_OUT_PRESENT)?    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: VK_IMAGE_LAYOUT_GENERAL)


static list      rendertargets = LIST_INIT(&rendertargets, NULL);
static graphics *gfx           = NULL;

static void reset(  gr_rendertarget *rt);
static void destroy(gr_rendertarget *rt);
static bool create_renderpass( gr_rendertarget *rt);
static bool create_framebuffer(gr_rendertarget *rt);



void gr_rendertarget_create(graphics *gr)
{

	gfx = gr;

}



void gr_rendertarget_destroy()
{

	while (!list_empty(&rendertargets))
		gr_rendertarget_del(list_front(&rendertargets));

	gfx = NULL;

}



gr_rendertarget *gr_rendertarget_new(const char *name)
{

	if (gr_rendertarget_find(name) != NULL)
		return NULL;

	gr_rendertarget *rt = malloc(sizeof(gr_rendertarget) + strlen(name) + 1);

	strcpy((char*)rt->name, name);
	reset(rt);

	list_init(  &rt->node, rt);
	list_append(&rendertargets, &rt->node);

	return rt;

}



void gr_rendertarget_del(gr_rendertarget *rt)
{

	if (rt == NULL)
		return;

	destroy(rt);

	list_remove(&rt->node);
	free(rt);

}



gr_rendertarget *gr_rendertarget_find(const char *name)
{

	for (list *rt=list_begin(&rendertargets); rt != list_end(&rendertargets); rt = rt->next)
		if (!strcmp(LIST_PTR(gr_rendertarget, rt)->name, name))
			return rt->ptr;

	return NULL;

}



int gr_rendertarget_append(gr_rendertarget *rt, VkImageView image, const VkClearValue *clear, int format, uint flags)
{

	if (rt->attachment_count >= GR_ATTACHMENTS_MAX)
		return -1;

	rt->attachment_image[rt->attachment_count]  = image;
	rt->attachment_format[rt->attachment_count] = format;
	rt->attachment_flags[rt->attachment_count]  = flags;

	if (clear != NULL)
		rt->attachment_clear[rt->attachment_count] = *clear;

	else
		szero(rt->attachment_clear[rt->attachment_count]);

	return rt->attachment_count++;

}



bool gr_rendertarget_build(gr_rendertarget *rt)
{

	destroy(rt);

	rt->renderpass  = gr_rendertarget_create_renderpass(rt);
	rt->framebuffer = gr_rendertarget_create_framebuffer(rt);

	if ((rt->renderpass == NULL) || (rt->framebuffer != NULL)) {

		destroy(rt);
		return false;

	}

	return true;

}



VkRenderPass gr_rendertarget_create_renderpass(const gr_rendertarget *rt)
{

	if (rt->attachment_count <= 0)
		return NULL;

	VkAttachmentDescription ad[GR_ATTACHMENTS_MAX]   = { 0 };
	VkAttachmentReference   ar_c[GR_ATTACHMENTS_MAX] = { 0 };
	VkAttachmentReference   ar_ds = { 0 };
	VkSubpassDescription    sd    = { 0 };
	VkRenderPassCreateInfo  rpci  = { 0 };

	int ar_c_num  = 0;
	int ar_ds_num = 0;

	for (int n=0; n < rt->attachment_count; n++) {

		ad[n].format         = rt->attachment_format[n];
		ad[n].samples        = VK_SAMPLE_COUNT_1_BIT;
		ad[n].loadOp         = LOAD_OP(rt->attachment_flags[n]);
		ad[n].storeOp        = STORE_OP(rt->attachment_flags[n]);
		ad[n].stencilLoadOp  = LOAD_OP(rt->attachment_flags[n]);
		ad[n].stencilStoreOp = STORE_OP(rt->attachment_flags[n]);
		ad[n].initialLayout  = LAYOUT_IN(rt->attachment_flags[n]);
		ad[n].finalLayout    = LAYOUT_OUT(rt->attachment_flags[n]);

		if (ad[n].finalLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL || //FIXME: This check can be done via the format, once we have pixelformats
		    ad[n].finalLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {

			ar_c[ar_c_num].attachment = n;
			ar_c[ar_c_num].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			ar_c_num++;

		} else if (ad[n].finalLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {

			ar_ds.attachment = n;
			ar_ds.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			ar_ds_num++;

		} else {

			//WTF do I do with this?

		}

	}

	sd.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
	sd.colorAttachmentCount    = ar_c_num;
	sd.pColorAttachments       = &ar_c[0];
	sd.pDepthStencilAttachment = (ar_ds_num > 0)? &ar_ds: NULL;

	rpci.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rpci.attachmentCount = rt->attachment_count;
	rpci.pAttachments    = &ad[0];
	rpci.subpassCount    = 1;
	rpci.pSubpasses      = &sd;

	VkRenderPass renderpass = NULL;

	if (vkCreateRenderPass(gfx->vk.gpu, &rpci, NULL, &renderpass) != VK_SUCCESS)
		return NULL;

	//rt->attachment_colors = ar_c_num;

	return renderpass;

}



VkFramebuffer gr_rendertarget_create_framebuffer(const gr_rendertarget *rt)
{

	if (rt->attachment_count <= 0 || rt->renderpass == NULL)
		return NULL;

	VkFramebufferCreateInfo fbci = {};

	fbci.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbci.renderPass      = rt->renderpass;
	fbci.attachmentCount = rt->attachment_count;
	fbci.pAttachments    = &rt->attachment_image[0];
	fbci.width           = rt->width;
	fbci.height          = rt->height;
	fbci.layers          = 1;

	VkFramebuffer framebuffer = NULL;

	if (vkCreateFramebuffer(gfx->vk.gpu, &fbci, NULL, &framebuffer) != VK_SUCCESS)
		return NULL;

	return framebuffer;

}



void reset(gr_rendertarget *rt)
{

	rt->renderpass  = NULL;
	rt->framebuffer = NULL;

	rt->width  = 0;
	rt->height = 0;

//	rt->attachment_colors = 0;
	rt->attachment_count  = 0;

	mzero(rt->attachment_image);
	mzero(rt->attachment_clear);
	mzero(rt->attachment_format);
	mzero(rt->attachment_flags);

}



void destroy(gr_rendertarget *rt)
{

	if (rt->framebuffer != NULL)
		vkDestroyFramebuffer(gfx->vk.gpu, rt->framebuffer, NULL);

	if (rt->renderpass != NULL)
		vkDestroyRenderPass(gfx->vk.gpu, rt->renderpass, NULL);

}

