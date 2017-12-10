

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/blob.h"
#include "core/logger.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/pixelformat.h"
#include "gr/framebuffer.h"
#include "gr/rendertarget.h"
#include "gr/surface.h"


static const VkFormat depth_formats[]={

	VK_FORMAT_D32_SFLOAT,
	VK_FORMAT_D32_SFLOAT_S8_UINT,
	VK_FORMAT_D24_UNORM_S8_UINT,
	VK_FORMAT_UNDEFINED

};

static graphics        *gfx   = NULL;
static gr_rendertarget *fbrt  = NULL;
static gr_surface      *depth = NULL;
static int              framebuffer_num = 0;
static VkFramebuffer    framebuffer_obj[GR_SWAPCHAIN_MAX] = { NULL };


static void reset();
static void destroy();
static bool create_depthbuffer();



void gr_framebuffer_create(graphics *gr)
{

	gfx  = gr;
	fbrt = gr_rendertarget_new("framebuffer");

}



void gr_framebuffer_destroy()
{

	destroy();
	gr_rendertarget_del(fbrt);

	fbrt = NULL;
	gfx  = NULL;

}



bool gr_framebuffer_init()
{

	fbrt->width  = gfx->vk.swapchain_width;
	fbrt->height = gfx->vk.swapchain_height;

	gr_rendertarget_append(
		fbrt,
		NULL,
		NULL,
		gfx->vk.swapchain_format,
		GR_RENDERPASS_LOAD_CLEAR | GR_RENDERPASS_STORE_PRESERVE | GR_RENDERPASS_LAYOUT_OUT_PRESENT);

	create_depthbuffer();

	fbrt->renderpass = gr_rendertarget_create_renderpass(fbrt);

	if (fbrt == NULL) {

		destroy();
		return false;

	}

	framebuffer_num = gfx->vk.swapchain_length;

	for (int n=0; n < framebuffer_num; n++) {

		fbrt->attachment_image[0] = gfx->vk.swapchain_views[n];
		framebuffer_obj[n]        = gr_rendertarget_create_framebuffer(fbrt);

		if (framebuffer_obj[n] == NULL) {

			destroy();
			return false;

		}

	}

	return true;

}



void gr_framebuffer_select()
{

	fbrt->framebuffer = framebuffer_obj[gfx->vk.swapchain_curr];

}



void reset()
{

/*
	color_bits   = 0;
	depth_bits   = 0;
	stencil_bits = 0;
*/
	depth = NULL;

	framebuffer_num = 0;

	mzero(framebuffer_obj);

	fbrt->framebuffer = NULL;
	fbrt->renderpass  = NULL;

	fbrt->attachment_count = 0;

}



void destroy()
{

	for (int n=0; n < framebuffer_num; n++)
		if (framebuffer_obj[n] != NULL)
			vkDestroyFramebuffer(gfx->vk.gpu, framebuffer_obj[n], NULL);

	if (fbrt->renderpass != NULL)
		vkDestroyRenderPass(gfx->vk.gpu, fbrt->renderpass, NULL);

	gr_surface_del(depth);

	reset();

}




bool create_depthbuffer()
{

	uint depth_format = VK_FORMAT_UNDEFINED;

	for (int n=0; depth_formats[n] != VK_FORMAT_UNDEFINED; n++) {

		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(gfx->vk.device, depth_formats[n], &props);

		if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {

			depth_format = depth_formats[n];
			break;

		}

	}

	if (depth_format == VK_FORMAT_UNDEFINED)
		return false;

	gr_surface_del(depth);

	depth = gr_surface_attachment("depthbuffer", fbrt->width, fbrt->height, depth_format, true);

	if (depth == NULL)
		return false;

	const VkClearValue depthstencil_clear = { 1.0f, 0 };

	gr_rendertarget_append(
		fbrt,
		depth->image_view,
		&depthstencil_clear,
		depth->pf->format,
		GR_RENDERPASS_LOAD_CLEAR | GR_RENDERPASS_LAYOUT_OUT_DS_OPTIMAL);

	return true;

}

