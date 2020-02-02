

#include "core/system.h"
#include "core/types.h"
#include "core/common.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/logger.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/pixelformat.h"
#include "gr/surface.h"
#include "gr/renderpass.h"



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
	((((x) & 0x0700) == GR_RENDERPASS_LAYOUT_OUT_C_OPTIMAL)?    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: \
	 (((x) & 0x0700) == GR_RENDERPASS_LAYOUT_OUT_DS_OPTIMAL)?   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: \
	 (((x) & 0x0700) == GR_RENDERPASS_LAYOUT_OUT_PRESENT)?      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: \
	 (((x) & 0x0700) == GR_RENDERPASS_LAYOUT_OUT_SHADER_READ)?  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: VK_IMAGE_LAYOUT_GENERAL)


static list      rendertargets = LIST_INIT(&rendertargets, NULL);
static graphics *gfx           = NULL;

static void reset(              gr_renderpass *rp);
static void destroy(            gr_renderpass *rp);
static bool create_renderpass(  gr_renderpass *rp);
static bool create_framebuffers(gr_renderpass *rp);



void gr_renderpass_create(graphics *gr)
{

	gfx = gr;

}



void gr_renderpass_destroy()
{

	while (!list_empty(&rendertargets))
		gr_renderpass_del(list_front(&rendertargets));

	gfx = NULL;

}



gr_renderpass *gr_renderpass_new(const char *name)
{

	if (gr_renderpass_find(name) != NULL)
		return NULL;

	gr_renderpass *rt = malloc(sizeof(gr_renderpass) + strlen(name) + 1);

	strcpy((char*)rt->name, name);
	reset(rt);

	list_init(  &rt->node, rt);
	list_append(&rendertargets, &rt->node);

	return rt;

}



void gr_renderpass_del(gr_renderpass *rp)
{

	if (rp == NULL)
		return;

	destroy(rp);

	list_remove(&rp->node);
	free(rp);

}



gr_renderpass *gr_renderpass_find(const char *name)
{

	for (list *rt=list_begin(&rendertargets); rt != list_end(&rendertargets); rt = rt->next)
		if (!strcmp(LIST_PTR(gr_renderpass, rt)->name, name))
			return rt->ptr;

	return NULL;

}



int gr_renderpass_attach(gr_renderpass *rp, VkImageView image, const VkClearValue *clear, int format, uint flags)
{

	if (rp->attachment_count >= GR_ATTACHMENTS_MAX)
		return -1;

	if (image == NULL)
		format = gfx->vk.swapchain_format;

	for (int n=0; n < gfx->vk.swapchain_length; n++)
		rp->attachment_image[n][rp->attachment_count] = (image == NULL)? gfx->vk.swapchain_views[n]: image;

	rp->attachment_format[rp->attachment_count] = format;
	rp->attachment_flags[rp->attachment_count]  = flags;

	if (clear != NULL)
		rp->attachment_clear[rp->attachment_count] = *clear;

	else
		szero(rp->attachment_clear[rp->attachment_count]);

	return rp->attachment_count++;

}



int gr_renderpass_surface(gr_renderpass *rp, gr_surface *surf)
{

	if (rp->surface_count >= GPU_SHADER_TEXTURES)
		return -1;

	if (!gr_surface_bind(surf))
		return -1;

	rp->surface_ids[rp->surface_count] = surf->id;

	return rp->surface_count++;

}



bool gr_renderpass_build(gr_renderpass *rp)
{

	destroy(rp);

	if (!create_renderpass(rp) || !create_framebuffers(rp)) {

		destroy(rp);
		return false;

	}

	return true;

}



void gr_renderpass_bind(gr_renderpass *rp, VkCommandBuffer cb)
{

}



bool create_renderpass(gr_renderpass *rp)
{

	if (rp->attachment_count <= 0)
		return false;

	VkAttachmentDescription ad[GR_ATTACHMENTS_MAX]   = { 0 };
	VkAttachmentReference   ar_c[GR_ATTACHMENTS_MAX] = { 0 };
	VkAttachmentReference   ar_ds   = { 0 };
	VkSubpassDescription    sd      = { 0 };
	VkRenderPassCreateInfo  rpci    = { 0 };
	VkSubpassDependency     deps[2] = { 0 };

	int ar_c_num  = 0;
	int ar_ds_num = 0;

	for (int n=0; n < rp->attachment_count; n++) {

		ad[n].format         = rp->attachment_format[n];
		ad[n].samples        = VK_SAMPLE_COUNT_1_BIT;
		ad[n].loadOp         = LOAD_OP(rp->attachment_flags[n]);
		ad[n].storeOp        = STORE_OP(rp->attachment_flags[n]);
		ad[n].stencilLoadOp  = LOAD_OP(rp->attachment_flags[n]);
		ad[n].stencilStoreOp = STORE_OP(rp->attachment_flags[n]);
		ad[n].initialLayout  = LAYOUT_IN(rp->attachment_flags[n]);
		ad[n].finalLayout    = LAYOUT_OUT(rp->attachment_flags[n]);

		if (ad[n].finalLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ||
		    ad[n].finalLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ||
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

	deps[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
	deps[0].dstSubpass      = 0;
	deps[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	deps[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	deps[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
	deps[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	deps[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	deps[1].srcSubpass      = 0;
	deps[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
	deps[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	deps[1].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	deps[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	deps[1].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
	deps[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	rpci.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rpci.attachmentCount = rp->attachment_count;
	rpci.pAttachments    = &ad[0];
	rpci.subpassCount    = 1;
	rpci.pSubpasses      = &sd;
	rpci.dependencyCount = countof(deps);
	rpci.pDependencies   = &deps[0];

	if (vkCreateRenderPass(gfx->vk.gpu, &rpci, NULL, &rp->renderpass) != VK_SUCCESS)
		return false;

	rp->attachment_colors = ar_c_num;

	return true;

}



bool create_framebuffers(gr_renderpass *rp)
{

	if (rp->attachment_count <= 0 || rp->renderpass == NULL)
		return NULL;

	for (int n=0; n < gfx->vk.swapchain_length; n++) {

		VkFramebufferCreateInfo fbci = { 0 };

		fbci.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbci.renderPass      = rp->renderpass;
		fbci.attachmentCount = rp->attachment_count;
		fbci.pAttachments    = &rp->attachment_image[n][0];
		fbci.width           = rp->width;
		fbci.height          = rp->height;
		fbci.layers          = 1;

		if (vkCreateFramebuffer(gfx->vk.gpu, &fbci, NULL, &rp->framebuffer[n]) != VK_SUCCESS)
			return false;

	}

	return true;

}



void reset(gr_renderpass *rp)
{

	rp->width  = 0;
	rp->height = 0;

	rp->attachment_colors = 0;
	rp->attachment_count  = 0;
	rp->surface_count     = 0;

	mzero(rp->attachment_image);
	mzero(rp->attachment_clear);
	mzero(rp->attachment_format);
	mzero(rp->attachment_flags);

	mzero(rp->surface_ids);

	rp->renderpass = NULL;

	mzero(rp->framebuffer);

}



void destroy(gr_renderpass *rp)
{

	for (int n=0; n < gfx->vk.swapchain_length; n++)
		if (rp->framebuffer[n] != NULL)
			vkDestroyFramebuffer(gfx->vk.gpu, rp->framebuffer[n], NULL);

	if (rp->renderpass != NULL)
		vkDestroyRenderPass(gfx->vk.gpu, rp->renderpass, NULL);

}

