

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/blob.h"
#include "core/logger.h"
#include "core/string.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/framebuffer.h"
#include "gr/rendertarget.h"


static const VkFormat depth_formats[]={

	VK_FORMAT_D32_SFLOAT,
	VK_FORMAT_D32_SFLOAT_S8_UINT,
	VK_FORMAT_D24_UNORM_S8_UINT,
	VK_FORMAT_UNDEFINED

};

static graphics        *gfx  = NULL;
static gr_rendertarget *fbrt = NULL;
static VkFormat         depthstencil_format;
static VkImage          depthstencil_image;
static VkDeviceMemory   depthstencil_memory;
static VkImageView      depthstencil_view;
static int              framebuffer_num;
static VkFramebuffer    framebuffer_obj[GR_SWAPCHAIN_MAX];


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

	//create_depthbuffer();

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
//FIXME: Post image transition!!
//FIXME: Other images need transition. move to common in graphics
#if 0
	VkImageMemoryBarrier imb;

	szero(imb);

	imb.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imb.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
	imb.newLayout                       = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	imb.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	imb.image                           = depthstencil_image;
	imb.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
	imb.subresourceRange.baseMipLevel   = 0;
	imb.subresourceRange.levelCount     = 1;
	imb.subresourceRange.baseArrayLayer = 0;
	imb.subresourceRange.layerCount     = 1;
	imb.srcAccessMask                   = 0;
	imb.dstAccessMask                   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	vkCmdPipelineBarrier(
		NULL,//gfx->vk.command_buffer_curr,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0, NULL,
		0, NULL,
		1, &imb);
#endif
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
	depthstencil_format = VK_FORMAT_UNDEFINED;
	depthstencil_image  = NULL;
	depthstencil_memory = NULL;
	depthstencil_view   = NULL;

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

	reset();

}




bool create_depthbuffer()
{

	depthstencil_format = VK_FORMAT_UNDEFINED;

	for (int n=0; depth_formats[n] != VK_FORMAT_UNDEFINED; n++) {

		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(gfx->vk.device, depth_formats[n], &props);

		if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {

			depthstencil_format = depth_formats[n];
			break;

		}

	}

	if (depthstencil_format == VK_FORMAT_UNDEFINED)
		return false;

	VkImageCreateInfo ici = {};

	ici.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ici.imageType     = VK_IMAGE_TYPE_2D;
	ici.extent.width  = fbrt->width;
	ici.extent.height = fbrt->height;
	ici.extent.depth  = 1;
	ici.mipLevels     = 1;
	ici.arrayLayers   = 1;
	ici.format        = depthstencil_format;
	ici.tiling        = VK_IMAGE_TILING_OPTIMAL;
	ici.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	ici.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	ici.samples       = VK_SAMPLE_COUNT_1_BIT;
	ici.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(gfx->vk.gpu, &ici, NULL, &depthstencil_image) != VK_SUCCESS)
		return false;

	VkMemoryRequirements mr;
	VkMemoryAllocateInfo mai = {};

	vkGetImageMemoryRequirements(gfx->vk.gpu, depthstencil_image, &mr);

	mai.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mai.allocationSize  = mr.size;
	mai.memoryTypeIndex = -1;//FIXME: rend->get_memory_type(mr.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(gfx->vk.gpu, &mai, NULL, &depthstencil_memory) != VK_SUCCESS)
		return false;

	vkBindImageMemory(gfx->vk.gpu, depthstencil_image, depthstencil_memory, 0);

	VkImageViewCreateInfo ivci;

	szero(ivci);

	ivci.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image                           = depthstencil_image;
	ivci.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
	ivci.format                          = depthstencil_format;
	ivci.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	ivci.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	ivci.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	ivci.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
	ivci.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
	ivci.subresourceRange.baseMipLevel   = 0;
	ivci.subresourceRange.levelCount     = 1;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount     = 1;

	if (vkCreateImageView(gfx->vk.gpu, &ivci, NULL, &depthstencil_view) != VK_SUCCESS)
		return false;

	const VkClearValue depthstencil_clear = { 1.0f, 0 };

	gr_rendertarget_append(
		fbrt,
		depthstencil_view,
		&depthstencil_clear,
		depthstencil_format,
		GR_RENDERPASS_LOAD_CLEAR | GR_RENDERPASS_LAYOUT_OUT_DS_OPTIMAL);


}

