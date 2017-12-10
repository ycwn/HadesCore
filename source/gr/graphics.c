

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/logger.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/vertexformat.h"
#include "gr/pixelformat.h"
#include "gr/framebuffer.h"
#include "gr/rendertarget.h"
#include "gr/surface.h"
#include "gr/vertexbuffer.h"
#include "gr/uniformbuffer.h"
#include "gr/shader.h"
#include "gr/command.h"
#include "gr/commandqueue.h"


GR_VKSYM_DEF(vkAllocateCommandBuffers);
GR_VKSYM_DEF(vkAllocateDescriptorSets);
GR_VKSYM_DEF(vkAllocateMemory);
GR_VKSYM_DEF(vkBeginCommandBuffer);
GR_VKSYM_DEF(vkBindBufferMemory);
GR_VKSYM_DEF(vkBindImageMemory);
GR_VKSYM_DEF(vkCmdBeginRenderPass);
GR_VKSYM_DEF(vkCmdBindDescriptorSets);
GR_VKSYM_DEF(vkCmdBindIndexBuffer);
GR_VKSYM_DEF(vkCmdBindPipeline);
GR_VKSYM_DEF(vkCmdBindVertexBuffers);
GR_VKSYM_DEF(vkCmdCopyBuffer);
GR_VKSYM_DEF(vkCmdCopyBufferToImage);
GR_VKSYM_DEF(vkCmdDraw);
GR_VKSYM_DEF(vkCmdDrawIndexed);
GR_VKSYM_DEF(vkCmdEndRenderPass);
GR_VKSYM_DEF(vkCmdPipelineBarrier);
GR_VKSYM_DEF(vkCreateBuffer);
GR_VKSYM_DEF(vkCreateCommandPool);
GR_VKSYM_DEF(vkCreateDescriptorPool);
GR_VKSYM_DEF(vkCreateDescriptorSetLayout);
GR_VKSYM_DEF(vkCreateDevice);
GR_VKSYM_DEF(vkCreateFence);
GR_VKSYM_DEF(vkCreateFramebuffer);
GR_VKSYM_DEF(vkCreateGraphicsPipelines);
GR_VKSYM_DEF(vkCreateImage);
GR_VKSYM_DEF(vkCreateImageView);
GR_VKSYM_DEF(vkCreateInstance);
GR_VKSYM_DEF(vkCreatePipelineLayout);
GR_VKSYM_DEF(vkCreateRenderPass);
GR_VKSYM_DEF(vkCreateSampler);
GR_VKSYM_DEF(vkCreateSemaphore);
GR_VKSYM_DEF(vkCreateShaderModule);
GR_VKSYM_DEF(vkDestroyBuffer);
GR_VKSYM_DEF(vkDestroyCommandPool);
GR_VKSYM_DEF(vkDestroyDescriptorPool);
GR_VKSYM_DEF(vkDestroyDescriptorSetLayout);
GR_VKSYM_DEF(vkDestroyDevice);
GR_VKSYM_DEF(vkDestroyFence);
GR_VKSYM_DEF(vkDestroyFramebuffer);
GR_VKSYM_DEF(vkDestroyImage);
GR_VKSYM_DEF(vkDestroyImageView);
GR_VKSYM_DEF(vkDestroyInstance);
GR_VKSYM_DEF(vkDestroyPipeline);
GR_VKSYM_DEF(vkDestroyPipelineLayout);
GR_VKSYM_DEF(vkDestroyRenderPass);
GR_VKSYM_DEF(vkDestroySampler);
GR_VKSYM_DEF(vkDestroySemaphore);
GR_VKSYM_DEF(vkDestroyShaderModule);
GR_VKSYM_DEF(vkEndCommandBuffer);
GR_VKSYM_DEF(vkEnumerateDeviceExtensionProperties);
GR_VKSYM_DEF(vkEnumerateInstanceExtensionProperties);
GR_VKSYM_DEF(vkEnumeratePhysicalDevices);
GR_VKSYM_DEF(vkFreeMemory);
GR_VKSYM_DEF(vkGetBufferMemoryRequirements);
GR_VKSYM_DEF(vkGetDeviceQueue);
GR_VKSYM_DEF(vkGetImageMemoryRequirements);
GR_VKSYM_DEF(vkGetInstanceProcAddr);
GR_VKSYM_DEF(vkGetPhysicalDeviceFeatures);
GR_VKSYM_DEF(vkGetPhysicalDeviceFormatProperties);
GR_VKSYM_DEF(vkGetPhysicalDeviceMemoryProperties);
GR_VKSYM_DEF(vkGetPhysicalDeviceProperties);
GR_VKSYM_DEF(vkGetPhysicalDeviceQueueFamilyProperties);
GR_VKSYM_DEF(vkMapMemory);
GR_VKSYM_DEF(vkQueueSubmit);
GR_VKSYM_DEF(vkQueueWaitIdle);
GR_VKSYM_DEF(vkResetFences);
GR_VKSYM_DEF(vkUnmapMemory);
GR_VKSYM_DEF(vkUpdateDescriptorSets);
GR_VKSYM_DEF(vkWaitForFences);

GR_VKSYM_DEF(vkAcquireNextImageKHR);
GR_VKSYM_DEF(vkCreateSwapchainKHR);
GR_VKSYM_DEF(vkDestroySurfaceKHR);
GR_VKSYM_DEF(vkDestroySwapchainKHR);
GR_VKSYM_DEF(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
GR_VKSYM_DEF(vkGetPhysicalDeviceSurfaceFormatsKHR);
GR_VKSYM_DEF(vkGetPhysicalDeviceSurfacePresentModesKHR);
GR_VKSYM_DEF(vkGetPhysicalDeviceSurfaceSupportKHR);
GR_VKSYM_DEF(vkGetSwapchainImagesKHR);
GR_VKSYM_DEF(vkQueuePresentKHR);
GR_VKSYM_DEF(vkCreateDebugReportCallbackEXT);
GR_VKSYM_DEF(vkDestroyDebugReportCallbackEXT);


#define GR_VKENUMERATE(name, N, T, func, ...)             \
	uint name##_num = 0;                              \
	T    name[(N)] = {};                              \
	do {                                              \
		func(__VA_ARGS__, &name##_num, NULL);     \
		watch("%d", name##_num);                  \
		if (name##_num > (N)) name##_num = (N);   \
		func(__VA_ARGS__, &name##_num, &name[0]); \
	} while (0)


#define GR_VKENUM_DEVICES(name, inst) \
	GR_VKENUMERATE(name, 16, VkPhysicalDevice, vkEnumeratePhysicalDevices, (inst));

#define GR_VKENUM_INSTANCE_EXT(name, layer) \
	GR_VKENUMERATE(name, 128, VkExtensionProperties, vkEnumerateInstanceExtensionProperties, (layer));

#define GR_VKENUM_DEVICE_EXT(name, dev, layer) \
	GR_VKENUMERATE(name, 128, VkExtensionProperties, vkEnumerateDeviceExtensionProperties, (dev), (layer));

#define GR_VKENUM_QUEUE_FAMILIES(name, dev) \
	GR_VKENUMERATE(name, 16, VkQueueFamilyProperties, vkGetPhysicalDeviceQueueFamilyProperties, (dev));

#define GR_VKENUM_SURFACE_FORMATS(name, dev, surf) \
	GR_VKENUMERATE(name, 512, VkSurfaceFormatKHR, vkGetPhysicalDeviceSurfaceFormatsKHR, (dev), (surf));

#define GR_VKENUM_PRESENT_MODES(name, dev, surf) \
	GR_VKENUMERATE(name, 8, VkPresentModeKHR, vkGetPhysicalDeviceSurfacePresentModesKHR, (dev), (surf));


static const char *validation_layers[]={
	"VK_LAYER_LUNARG_standard_validation"
};

static const char *validation_extensions[]={
	"VK_EXT_debug_report"
};

static const char *instance_extensions[]={
};

static const char *device_extensions[]={
	"VK_KHR_swapchain"
};


static graphics gfx = { 0 };


static void reset();
static void destroy();
static bool init_vulkan();
static bool init_device();
static bool init_swapchain();
static bool init_pipeline();
static bool init_commandpool();
static bool init_synchronization();
static bool init_descriptors();

static VKAPI_ATTR VkBool32 VKAPI_CALL debug(
		VkDebugReportFlagsEXT      flags,
		VkDebugReportObjectTypeEXT obj_type,
		uint64_t                   obj,
		size_t                     location,
		int32_t                    code,
		const char                 *layer_prefix,
		const char                 *msg,
		void                       *ptr);


static inline bool fail_msg(const char *msg) {
	log_d("%s", msg);
	return false;
}



graphics *gr_create()
{

	gfx.var.screen_width      = var_new_int("gr.screen.width",   "1024");
	gfx.var.screen_height     = var_new_int("gr.screen.height",   "576");
	gfx.var.screen_fullscreen = var_new_int("gr.screen.full",       "0");
	gfx.var.vsync_enable      = var_new_int("gr.vsync.enable",      "1");
	gfx.var.vsync_adaptive    = var_new_int("gr.vsync.adaptive",    "1");
	gfx.var.triple_buffer     = var_new_int("gr.tribuffer",         "0");
	gfx.var.validate          = var_new_int("gr.validate",          "0");

	gfx.ext.instance_num = 0;
	gfx.ext.device_num   = 0;

	mzero(gfx.ext.instance);
	mzero(gfx.ext.device);

	reset();

	gr_pixelformat_create(&gfx);
	gr_rendertarget_create(&gfx);
	gr_framebuffer_create(&gfx);
	gr_shader_create(&gfx);
	gr_surface_create(&gfx);
	gr_command_create();
	gr_commandqueue_create();
	gr_vertexbuffer_create(&gfx);
	gr_uniformbuffer_create(&gfx);

	for (int n=0; n < countof(instance_extensions); n++)
		gr_request_instance_extension(instance_extensions[n]);

	for (int n=0; n < countof(device_extensions); n++)
		gr_request_device_extension(device_extensions[n]);

	return &gfx;

}



void gr_destroy()
{

	gr_uniformbuffer_destroy();
	gr_vertexbuffer_destroy();
	gr_commandqueue_destroy();
	gr_command_destroy();
	gr_surface_destroy();
	gr_shader_destroy();
	gr_framebuffer_destroy();
	gr_rendertarget_destroy();
	gr_pixelformat_destroy();

	destroy();

	var_del(gfx.var.screen_width);
	var_del(gfx.var.screen_height);
	var_del(gfx.var.screen_fullscreen);
	var_del(gfx.var.vsync_enable);
	var_del(gfx.var.vsync_adaptive);
	var_del(gfx.var.triple_buffer);
	var_del(gfx.var.validate);

}



bool gr_request_instance_extension(const char *ext)
{

	for (int n=0; n < gfx.ext.instance_num; n++)
		if (!strcmp(ext, gfx.ext.instance[n]))
			return true;

	if (gfx.ext.instance_num >= GR_INSTANCE_EXT_MAX)
		return false;

	gfx.ext.instance[gfx.ext.instance_num++] = ext;

	return true;

}



bool gr_request_device_extension(const char *ext)
{

	for (int n=0; n < gfx.ext.device_num; n++)
		if (!strcmp(ext, gfx.ext.device[n]))
			return true;

	if (gfx.ext.device_num >= GR_DEVICE_EXT_MAX)
		return false;

	gfx.ext.device[gfx.ext.device_num++] = ext;

	return true;

}



bool gr_set_video()
{

	destroy();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE,  GLFW_FALSE);

	gfx.window = glfwCreateWindow(
			*gfx.var.screen_width, *gfx.var.screen_height,
			"Hades Core",
			*gfx.var.screen_fullscreen? glfwGetPrimaryMonitor(): NULL, NULL);

	watch("%p", gfx.window);

	if (gfx.window == NULL ||
		!init_vulkan()      || !init_device()          || !init_swapchain()   ||
		!init_commandpool() || !init_synchronization() || !init_descriptors() ||

		!gr_pixelformat_init() ||
		!gr_framebuffer_init()) {

		log_e("graphics: Graphics initialization sequence failed");
		destroy();

		return false;

	}

	log_i("graphics: Graphics initialization sequence complete");

	return true;

}



void gr_submit()
{

	vkAcquireNextImageKHR(
		gfx.vk.gpu,
		gfx.vk.swapchain, 1000000000,
		gfx.vk.signal_image_ready, NULL, (uint*)&gfx.vk.swapchain_curr);

	gr_commandqueue_consume();
	gr_framebuffer_select();
	gr_surface_update_cache();

	VkCommandBuffer   curr_cmd    = gfx.vk.command_buffer[gfx.vk.swapchain_curr];
	VkFence           curr_fence  = gfx.vk.command_fence[gfx.vk.swapchain_curr];
	gr_rendertarget  *curr_target = NULL;
	gr_shader        *curr_shader = NULL;
	gr_vertexbuffer  *curr_vbo    = NULL;
	gr_uniformbuffer *curr_ubo    = NULL;
	//core::surface   *curr_tex    = NULL;

	if (vkWaitForFences(gfx.vk.gpu, 1, &curr_fence, VK_TRUE, 1000000000) != VK_SUCCESS)
		return;

	if (vkResetFences(gfx.vk.gpu, 1, &curr_fence) != VK_SUCCESS)
		return;

	const VkDeviceSize       zero = 0;
	VkCommandBufferBeginInfo cbbi = { 0 };

	cbbi.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbbi.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	cbbi.pInheritanceInfo = NULL;

	vkBeginCommandBuffer(curr_cmd, &cbbi);

	for (const gr_command **cq=gr_commandqueue_begin(); cq != gr_commandqueue_end(); cq++) {

		const gr_command *cmd = *cq;

		if (curr_target != cmd->shader->rt) {

			if (curr_target != NULL)
				vkCmdEndRenderPass(curr_cmd);

			curr_target = cmd->shader->rt;
			curr_shader = NULL;

			VkRenderPassBeginInfo rpi = { 0 };

			rpi.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			rpi.renderPass               = curr_target->renderpass;
			rpi.framebuffer              = curr_target->framebuffer;
			rpi.renderArea.offset.x      = 0;
			rpi.renderArea.offset.y      = 0;
			rpi.renderArea.extent.width  = curr_target->width;
			rpi.renderArea.extent.height = curr_target->height;
			rpi.clearValueCount          = curr_target->attachment_count;
			rpi.pClearValues             = curr_target->attachment_clear;

			vkCmdBeginRenderPass(curr_cmd, &rpi, VK_SUBPASS_CONTENTS_INLINE);

		}

		if (curr_shader != cmd->shader) {

			curr_shader = cmd->shader;
			curr_vbo    = NULL;
			curr_ubo    = NULL;
//			curr_tex    = NULL;

			vkCmdBindPipeline(curr_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, curr_shader->pipeline);

		}

		if (curr_vbo != cmd->vertices) {

			curr_vbo = cmd->vertices;

			if (curr_vbo->vertex.buffer != NULL)
				vkCmdBindVertexBuffers(curr_cmd, 0, 1, &curr_vbo->vertex.buffer, &zero);

			if (curr_vbo->index.buffer != NULL)
				vkCmdBindIndexBuffer(curr_cmd, curr_vbo->index.buffer, 0, VK_INDEX_TYPE_UINT16);

		}

		if (curr_ubo != cmd->uniforms/*|| curr_tex != cmd->t*/) {

			curr_ubo = cmd->uniforms;
			curr_mat = cmd->material;

			const VkDescriptorSet descriptors[]={
				gr_surface_get_descriptor(),
				curr_shader->ub.descriptor,
				curr_ubo->descriptor
			};

			vkCmdBindDescriptorSets(
				curr_cmd,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				gfx.vk.pipeline_layout,
				0, countof(descriptors), &descriptors[0],
				0, NULL);

		}

		if (curr_vbo->index.buffer != NULL)
			vkCmdDrawIndexed(curr_cmd, cmd->count, 1, 0, 0, 0);

		else
			vkCmdDraw(curr_cmd, cmd->count, 1, 0, 0);

	}

	if (curr_target != NULL)
		vkCmdEndRenderPass(curr_cmd);

	if (vkEndCommandBuffer(curr_cmd) != VK_SUCCESS)
		return;

	VkSubmitInfo     si = { 0 };
	VkPresentInfoKHR pi = { 0 };

	si.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	si.waitSemaphoreCount   = 1;
	si.pWaitSemaphores      = &gfx.vk.signal_image_ready;
	si.pWaitDstStageMask    = &(const VkPipelineStageFlags){ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	si.commandBufferCount   = 1;
	si.pCommandBuffers      = &gfx.vk.command_buffer[gfx.vk.swapchain_curr];
	si.signalSemaphoreCount = 1;
	si.pSignalSemaphores    = &gfx.vk.signal_render_complete;

	vkQueueSubmit(gfx.vk.graphics_queue, 1, &si, curr_fence);

	pi.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	pi.waitSemaphoreCount = 1;
	pi.pWaitSemaphores    = &gfx.vk.signal_render_complete;
	pi.swapchainCount     = 1;
	pi.pSwapchains        = &gfx.vk.swapchain;
	pi.pImageIndices      = (uint*)&gfx.vk.swapchain_curr;
	pi.pResults           = NULL;

	vkQueuePresentKHR(gfx.vk.presentation_queue, &pi);

}



int gr_get_memory_type(uint mask, uint props)
{

	for (int n=0; n < gfx.vk.memory_properties.memoryTypeCount; n++)
		if ((mask & (1 << n)) && (gfx.vk.memory_properties.memoryTypes[n].propertyFlags & props) == props)
			return n;

	return -1;

}



bool gr_create_buffer(VkBuffer *buf, VkDeviceMemory *mem, size_t size, uint usage, uint props)
{

	*buf = NULL;
	*mem = NULL;

        VkBufferCreateInfo   bci = { 0 };
	VkMemoryAllocateInfo mai = { 0 };
	VkMemoryRequirements mr;

	bci.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bci.size        = size;
	bci.usage       = usage;
	bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(gfx.vk.gpu, &bci, NULL, buf) != VK_SUCCESS)
		goto fail;

	vkGetBufferMemoryRequirements(gfx.vk.gpu, *buf, &mr);

	mai.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mai.allocationSize  = mr.size;
	mai.memoryTypeIndex = gr_get_memory_type(mr.memoryTypeBits, props);

        if (vkAllocateMemory(gfx.vk.gpu, &mai, NULL, mem) != VK_SUCCESS)
		goto fail;

        vkBindBufferMemory(gfx.vk.gpu, *buf, *mem, 0);
	return true;

fail:
	if (*mem != NULL)
		vkFreeMemory(gfx.vk.gpu, *mem, NULL);

	if (*buf != NULL)
		vkDestroyBuffer(gfx.vk.gpu, *buf, NULL);

	*buf = NULL;
	*mem = NULL;

	return false;

}



bool gr_create_image(
		VkImage        *img,
		VkDeviceMemory *mem,
		VkFormat        format,
		uint            width,   uint height, uint depth,
		uint            mipmaps, uint layers,
		uint            tiling,  uint usage,  uint props, uint flags)
{

	*img = NULL;
	*mem = NULL;

	VkImageCreateInfo    ici = { 0 };
	VkMemoryAllocateInfo mai = { 0 };
	VkMemoryRequirements mr;

	ici.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ici.imageType     = VK_IMAGE_TYPE_2D;
	ici.extent.width  = width;
	ici.extent.height = height;
	ici.extent.depth  = depth;
	ici.mipLevels     = mipmaps;
	ici.arrayLayers   = layers;
	ici.format        = format;
	ici.tiling        = tiling;
	ici.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	ici.usage         = usage;
	ici.flags         = flags;
	ici.samples       = VK_SAMPLE_COUNT_1_BIT;
	ici.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(gfx.vk.gpu, &ici, NULL, img) != VK_SUCCESS)
		goto fail;

	vkGetImageMemoryRequirements(gfx.vk.gpu, *img, &mr);

	mai.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mai.allocationSize  = mr.size;
	mai.memoryTypeIndex = gr_get_memory_type(mr.memoryTypeBits, props);

	if (vkAllocateMemory(gfx.vk.gpu, &mai, NULL, mem) != VK_SUCCESS)
		goto fail;

	vkBindImageMemory(gfx.vk.gpu, *img, *mem, 0);
	return true;

fail:
	if (*mem != NULL)
		vkFreeMemory(gfx.vk.gpu, *mem, NULL);

	if (*img != NULL)
		vkDestroyImage(gfx.vk.gpu, *img, NULL);

	*img = NULL;
	*mem = NULL;

	return false;

}



bool gr_create_image_view(VkImageView *view, VkImage img, VkFormat fmt, VkImageAspectFlags asp) //TODO: Move this into surface
{

	VkImageViewCreateInfo ivci = { 0 };

	ivci.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image                           = img;
	ivci.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
	ivci.format                          = fmt;
	ivci.subresourceRange.aspectMask     = asp;
	ivci.subresourceRange.baseMipLevel   = 0;
	ivci.subresourceRange.levelCount     = VK_REMAINING_MIP_LEVELS;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount     = 1;

	return vkCreateImageView(gfx.vk.gpu, &ivci, NULL, view) == VK_SUCCESS;

}



void gr_transfer_begin()
{

	VkCommandBufferBeginInfo cbbi = { 0 };

	cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(gfx.vk.transfer_buffer, &cbbi);

}



void gr_transfer_end()
{

	vkEndCommandBuffer(gfx.vk.transfer_buffer);

	VkSubmitInfo si = { 0 };

	si.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	si.commandBufferCount = 1;
	si.pCommandBuffers    = &gfx.vk.transfer_buffer;

	vkQueueSubmit(  gfx.vk.transfer_queue, 1, &si, NULL);
	vkQueueWaitIdle(gfx.vk.transfer_queue);

}



void gr_upload(const void *src, VkDeviceMemory dst, size_t len)
{

	void *data = NULL;

	vkMapMemory(gfx.vk.gpu, dst, 0, len, 0, &data);

        memcpy(data, src, len);

	vkUnmapMemory(gfx.vk.gpu, dst);

}



void gr_copy_buffer_to_buffer(VkBuffer src, VkBuffer dst, size_t len)
{

	gr_transfer_begin();

	vkCmdCopyBuffer(gfx.vk.transfer_buffer, src, dst, 1, &(VkBufferCopy){ 0, 0, len });

	gr_transfer_end();

}



void gr_copy_buffer_to_image(VkBuffer src, VkImage dst, uint width, uint height, uint depth)
{

	gr_transfer_begin();

	VkBufferImageCopy bic = { 0 };

	bic.bufferOffset      = 0;
	bic.bufferRowLength   = 0;
	bic.bufferImageHeight = 0;
	bic.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	bic.imageSubresource.mipLevel       = 0;
	bic.imageSubresource.baseArrayLayer = 0;
	bic.imageSubresource.layerCount     = 1;
	bic.imageOffset.x      = 0;
	bic.imageOffset.y      = 0;
	bic.imageOffset.z      = 0;
	bic.imageExtent.width  = width;
	bic.imageExtent.height = height;
	bic.imageExtent.depth  = depth;

	vkCmdCopyBufferToImage(gfx.vk.transfer_buffer, src, dst,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bic);

	gr_transfer_end();

}



void gr_mcopy_buffer_to_buffer(VkBuffer src, VkBuffer dst, uint skip[], uint offset[], size_t length[], uint count)
{

	gr_transfer_begin();

	VkBufferCopy bc[count];

	for (int n=0; n < count; n++) {

		bc[n].srcOffset = skip[n];
		bc[n].dstOffset = offset[n];
		bc[n].size      = length[n];

	}

	vkCmdCopyBuffer(gfx.vk.transfer_buffer, src, dst, count, &bc[0]);

	gr_transfer_end();

}



void gr_mcopy_buffer_to_image(
		VkBuffer src, VkImage dst,
		uint width,    uint height,   uint depth,
		uint offset[], uint mipmap[], uint layer[],
		uint count)
{

	gr_transfer_begin();

	VkBufferImageCopy bic[count];

	for (int n=0; n < count; n++) {

		bic[n].bufferOffset      = offset[n];
		bic[n].bufferRowLength   = 0;
		bic[n].bufferImageHeight = 0;
		bic[n].imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		bic[n].imageSubresource.mipLevel       = mipmap[n];
		bic[n].imageSubresource.baseArrayLayer = layer[n];
		bic[n].imageSubresource.layerCount     = 1;
		bic[n].imageOffset.x      = 0;
		bic[n].imageOffset.y      = 0;
		bic[n].imageOffset.z      = 0;
		bic[n].imageExtent.width  = maxu(width  >> mipmap[n], 1);
		bic[n].imageExtent.height = maxu(height >> mipmap[n], 1);
		bic[n].imageExtent.depth  = maxu(depth  >> mipmap[n], 1);

	}

	vkCmdCopyBufferToImage(gfx.vk.transfer_buffer, src, dst,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, count, &bic[0]);

	gr_transfer_end();

}



void gr_transition_layout(VkImage img, VkImageLayout layout, VkImageAspectFlags mask, VkAccessFlagBits src, VkAccessFlagBits dst)
{

	gr_transfer_begin();

	VkImageMemoryBarrier imb = { 0 };

	imb.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imb.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
	imb.newLayout                       = layout;
	imb.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	imb.image                           = img;
	imb.subresourceRange.aspectMask     = mask;
	imb.subresourceRange.baseMipLevel   = 0;
	imb.subresourceRange.levelCount     = VK_REMAINING_MIP_LEVELS;
	imb.subresourceRange.baseArrayLayer = 0;
	imb.subresourceRange.layerCount     = 1;
	imb.srcAccessMask                   = src;
	imb.dstAccessMask                   = dst;

	VkPipelineStageFlags src_mask =
		(src == VK_ACCESS_INDIRECT_COMMAND_READ_BIT)?           VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT:
		(src == VK_ACCESS_INDEX_READ_BIT)?                      VK_PIPELINE_STAGE_VERTEX_INPUT_BIT:
		(src == VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)?           VK_PIPELINE_STAGE_VERTEX_INPUT_BIT:
		(src == VK_ACCESS_UNIFORM_READ_BIT)?                    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT:
		(src == VK_ACCESS_INPUT_ATTACHMENT_READ_BIT)?           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT:
		(src == VK_ACCESS_SHADER_READ_BIT)?                     VK_PIPELINE_STAGE_VERTEX_SHADER_BIT:
		(src == VK_ACCESS_SHADER_WRITE_BIT)?                    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT:
		(src == VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)?           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT:
		(src == VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)?          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT:
		(src == VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT)?   VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT:
		(src == VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)?  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT:
		(src == VK_ACCESS_TRANSFER_READ_BIT)?                   VK_PIPELINE_STAGE_TRANSFER_BIT:
		(src == VK_ACCESS_TRANSFER_WRITE_BIT)?                  VK_PIPELINE_STAGE_TRANSFER_BIT:
		(src == VK_ACCESS_HOST_READ_BIT)?                       VK_PIPELINE_STAGE_HOST_BIT:
		(src == VK_ACCESS_HOST_WRITE_BIT)?                      VK_PIPELINE_STAGE_HOST_BIT:
									VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	VkPipelineStageFlags dst_mask =
		(dst == VK_ACCESS_INDIRECT_COMMAND_READ_BIT)?           VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT:
		(dst == VK_ACCESS_INDEX_READ_BIT)?                      VK_PIPELINE_STAGE_VERTEX_INPUT_BIT:
		(dst == VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)?           VK_PIPELINE_STAGE_VERTEX_INPUT_BIT:
		(dst == VK_ACCESS_UNIFORM_READ_BIT)?                    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT:
		(dst == VK_ACCESS_INPUT_ATTACHMENT_READ_BIT)?           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT:
		(dst == VK_ACCESS_SHADER_READ_BIT)?                     VK_PIPELINE_STAGE_VERTEX_SHADER_BIT:
		(dst == VK_ACCESS_SHADER_WRITE_BIT)?                    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT:
		(dst == VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)?           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT:
		(dst == VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)?          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT:
		(dst == VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT)?   VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT:
		(dst == VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)?  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT:
		(dst == VK_ACCESS_TRANSFER_READ_BIT)?                   VK_PIPELINE_STAGE_TRANSFER_BIT:
		(dst == VK_ACCESS_TRANSFER_WRITE_BIT)?                  VK_PIPELINE_STAGE_TRANSFER_BIT:
		(dst == VK_ACCESS_HOST_READ_BIT)?                       VK_PIPELINE_STAGE_HOST_BIT:
		(dst == VK_ACCESS_HOST_WRITE_BIT)?                      VK_PIPELINE_STAGE_HOST_BIT:
									VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier(gfx.vk.transfer_buffer, src_mask, dst_mask, 0, 0, NULL, 0, NULL, 1, &imb);

	gr_transfer_end();

}



void reset()
{

	gfx.window = NULL;

	gfx.vk.instance = NULL;
	gfx.vk.device   = NULL;
	gfx.vk.gpu      = NULL;

 	memset(&gfx.vk.memory_properties, 0, sizeof(gfx.vk.memory_properties));

	gfx.vk.graphics_queue_index = -1;
	gfx.vk.graphics_queue_count =  0;
	gfx.vk.graphics_queue       = NULL;

	gfx.vk.compute_queue_index = -1;
	gfx.vk.compute_queue_count =  0;
	gfx.vk.compute_queue       = NULL;

	gfx.vk.transfer_queue_index = -1;
	gfx.vk.transfer_queue_count =  0;
	gfx.vk.transfer_queue       = NULL;

	gfx.vk.presentation_queue_index = -1;
	gfx.vk.presentation_queue_count =  0;
	gfx.vk.presentation_queue       = NULL;

	gfx.vk.surface = NULL;

	gfx.vk.swapchain_width  = 0;
	gfx.vk.swapchain_height = 0;
	gfx.vk.swapchain_length = 0;
	gfx.vk.swapchain_curr   = -1;
	gfx.vk.swapchain_format = VK_FORMAT_UNDEFINED;
	gfx.vk.swapchain        = NULL;

	gfx.vk.command_pool = NULL;

	mzero(gfx.vk.command_buffer);
	mzero(gfx.vk.command_fence);

	gfx.vk.transfer_pool   = NULL;
	gfx.vk.transfer_buffer = NULL;

	gfx.vk.signal_image_ready     = NULL;
	gfx.vk.signal_render_complete = NULL;

	gfx.vk.descriptor_uniform_pool = NULL;
	gfx.vk.descriptor_texture_pool = NULL;

	gfx.vk.descriptor_uniform_layout = NULL;
	gfx.vk.descriptor_texture_layout = NULL;

	gfx.vk.pipeline_layout = NULL;

}



void destroy()
{

	if (gfx.vk.pipeline_layout != NULL)
		vkDestroyPipelineLayout(gfx.vk.gpu, gfx.vk.pipeline_layout, NULL);

	if (gfx.vk.descriptor_uniform_layout != NULL)
		vkDestroyDescriptorSetLayout(gfx.vk.gpu, gfx.vk.descriptor_uniform_layout, NULL);

	if (gfx.vk.descriptor_texture_layout != NULL)
		vkDestroyDescriptorSetLayout(gfx.vk.gpu, gfx.vk.descriptor_texture_layout, NULL);

	if (gfx.vk.descriptor_uniform_pool != NULL)
		vkDestroyDescriptorPool(gfx.vk.gpu, gfx.vk.descriptor_uniform_pool, NULL);

	if (gfx.vk.descriptor_texture_pool != NULL)
		vkDestroyDescriptorPool(gfx.vk.gpu, gfx.vk.descriptor_texture_pool, NULL);

	for (int n=0; n < gfx.vk.swapchain_length; n++)
		if (gfx.vk.command_fence[n] != NULL)
			vkDestroyFence(gfx.vk.gpu, gfx.vk.command_fence[n], NULL);

	if (gfx.vk.signal_render_complete != NULL)
		vkDestroySemaphore(gfx.vk.gpu, gfx.vk.signal_render_complete, NULL);

	if (gfx.vk.signal_image_ready != NULL)
		vkDestroySemaphore(gfx.vk.gpu, gfx.vk.signal_image_ready, NULL);

	if (gfx.vk.command_pool != NULL)
		vkDestroyCommandPool(gfx.vk.gpu, gfx.vk.command_pool, NULL);

	if (gfx.vk.transfer_pool != NULL)
		vkDestroyCommandPool(gfx.vk.gpu, gfx.vk.transfer_pool, NULL);

	if (gfx.vk.swapchain != NULL) {

		for (int n=0; n < gfx.vk.swapchain_length; n++)
			vkDestroyImageView(gfx.vk.gpu, gfx.vk.swapchain_views[n], NULL);

		vkDestroySwapchainKHR(gfx.vk.gpu, gfx.vk.swapchain, NULL);

	}

	if (gfx.vk.gpu != NULL)
		vkDestroyDevice(gfx.vk.gpu, NULL);

	if (gfx.vk.surface != NULL)
		vkDestroySurfaceKHR(gfx.vk.instance, gfx.vk.surface, NULL);

	if (gfx.vk.callback != NULL)
		vkDestroyDebugReportCallbackEXT(gfx.vk.instance, gfx.vk.callback, NULL);

	if (gfx.vk.instance != NULL)
		vkDestroyInstance(gfx.vk.instance, NULL);

	if (gfx.window != NULL)
		glfwDestroyWindow(gfx.window);

	reset();

}



bool init_vulkan()
{

	if (!glfwVulkanSupported())
		return fail_msg("graphics: Vulkan is not supported!");

	GR_VKSYM(vkAllocateCommandBuffers);
	GR_VKSYM(vkAllocateDescriptorSets);
	GR_VKSYM(vkAllocateMemory);
	GR_VKSYM(vkBeginCommandBuffer);
	GR_VKSYM(vkBindBufferMemory);
	GR_VKSYM(vkBindImageMemory);
	GR_VKSYM(vkCmdBeginRenderPass);
	GR_VKSYM(vkCmdBindDescriptorSets);
	GR_VKSYM(vkCmdBindIndexBuffer);
	GR_VKSYM(vkCmdBindPipeline);
	GR_VKSYM(vkCmdBindVertexBuffers);
	GR_VKSYM(vkCmdCopyBuffer);
	GR_VKSYM(vkCmdCopyBufferToImage);
	GR_VKSYM(vkCmdDraw);
	GR_VKSYM(vkCmdDrawIndexed);
	GR_VKSYM(vkCmdEndRenderPass);
	GR_VKSYM(vkCmdPipelineBarrier);
	GR_VKSYM(vkCreateBuffer);
	GR_VKSYM(vkCreateCommandPool);
	GR_VKSYM(vkCreateDescriptorPool);
	GR_VKSYM(vkCreateDescriptorSetLayout);
	GR_VKSYM(vkCreateDevice);
	GR_VKSYM(vkCreateFence);
	GR_VKSYM(vkCreateFramebuffer);
	GR_VKSYM(vkCreateGraphicsPipelines);
	GR_VKSYM(vkCreateImage);
	GR_VKSYM(vkCreateImageView);
	GR_VKSYM(vkCreateInstance);
	GR_VKSYM(vkCreatePipelineLayout);
	GR_VKSYM(vkCreateRenderPass);
	GR_VKSYM(vkCreateSampler);
	GR_VKSYM(vkCreateSemaphore);
	GR_VKSYM(vkCreateShaderModule);
	GR_VKSYM(vkDestroyBuffer);
	GR_VKSYM(vkDestroyCommandPool);
	GR_VKSYM(vkDestroyDescriptorPool);
	GR_VKSYM(vkDestroyDescriptorSetLayout);
	GR_VKSYM(vkDestroyDevice);
	GR_VKSYM(vkDestroyFence);
	GR_VKSYM(vkDestroyFramebuffer);
	GR_VKSYM(vkDestroyImage);
	GR_VKSYM(vkDestroyImageView);
	GR_VKSYM(vkDestroyInstance);
	GR_VKSYM(vkDestroyPipeline);
	GR_VKSYM(vkDestroyPipelineLayout);
	GR_VKSYM(vkDestroyRenderPass);
	GR_VKSYM(vkDestroySampler);
	GR_VKSYM(vkDestroySemaphore);
	GR_VKSYM(vkDestroyShaderModule);
	GR_VKSYM(vkEndCommandBuffer);
	GR_VKSYM(vkEnumerateDeviceExtensionProperties);
	GR_VKSYM(vkEnumerateInstanceExtensionProperties);
	GR_VKSYM(vkEnumeratePhysicalDevices);
	GR_VKSYM(vkFreeMemory);
	GR_VKSYM(vkGetBufferMemoryRequirements);
	GR_VKSYM(vkGetDeviceQueue);
	GR_VKSYM(vkGetImageMemoryRequirements);
	GR_VKSYM(vkGetPhysicalDeviceFeatures);
	GR_VKSYM(vkGetPhysicalDeviceFormatProperties);
	GR_VKSYM(vkGetPhysicalDeviceMemoryProperties);
	GR_VKSYM(vkGetPhysicalDeviceProperties);
	GR_VKSYM(vkGetPhysicalDeviceQueueFamilyProperties);
	GR_VKSYM(vkMapMemory);
	GR_VKSYM(vkQueueSubmit);
	GR_VKSYM(vkQueueWaitIdle);
	GR_VKSYM(vkResetFences);
	GR_VKSYM(vkUnmapMemory);
	GR_VKSYM(vkUpdateDescriptorSets);
	GR_VKSYM(vkWaitForFences);

	uint         glfw_ext_num   = 0;
	const char **glfw_ext_names = glfwGetRequiredInstanceExtensions(&glfw_ext_num);

	for (int n=0; n < glfw_ext_num; n++)
		gr_request_instance_extension(glfw_ext_names[n]);

	if (*gfx.var.validate)
		for (int n=0; n < countof(validation_extensions); n++)
			gr_request_instance_extension(validation_extensions[n]);

	VkApplicationInfo    vkai  = {};
	VkInstanceCreateInfo vkici = {};

	vkai.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vkai.pApplicationName   = "Katabasis";
	vkai.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	vkai.pEngineName        = "Hades Core";
	vkai.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
	vkai.apiVersion         = VK_API_VERSION_1_0;

	vkici.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkici.pApplicationInfo        = &vkai;
	vkici.enabledExtensionCount   = gfx.ext.instance_num;
	vkici.ppEnabledExtensionNames = gfx.ext.instance;
	vkici.enabledLayerCount       = *gfx.var.validate? countof(validation_layers): 0;
	vkici.ppEnabledLayerNames     = *gfx.var.validate? validation_layers: NULL;

	if (vkCreateInstance(&vkici, NULL, &gfx.vk.instance) != VK_SUCCESS)
		return fail_msg("graphics: vkCreateInstance() failed!");

	log_i("graphics: Vulkan initialized");

	GR_VKSYM(vkAcquireNextImageKHR);
	GR_VKSYM(vkCreateSwapchainKHR);
	GR_VKSYM(vkDestroySurfaceKHR);
	GR_VKSYM(vkDestroySwapchainKHR);
	GR_VKSYM(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
	GR_VKSYM(vkGetPhysicalDeviceSurfaceFormatsKHR);
	GR_VKSYM(vkGetPhysicalDeviceSurfacePresentModesKHR);
	GR_VKSYM(vkGetPhysicalDeviceSurfaceSupportKHR);
	GR_VKSYM(vkGetSwapchainImagesKHR);
	GR_VKSYM(vkQueuePresentKHR);
	GR_VKSYM(vkCreateDebugReportCallbackEXT);
	GR_VKSYM(vkDestroyDebugReportCallbackEXT);

	if (*gfx.var.validate) {

		VkDebugReportCallbackCreateInfoEXT vkdrcci = {};

		vkdrcci.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		vkdrcci.flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		vkdrcci.pfnCallback = debug;

		if (vkCreateDebugReportCallbackEXT(gfx.vk.instance, &vkdrcci, NULL, &gfx.vk.callback) != VK_SUCCESS)
			log_e("graphics: warning: Failed to set up debug callback!");

	}

	if (glfwCreateWindowSurface(gfx.vk.instance, gfx.window, NULL, &gfx.vk.surface) != VK_SUCCESS)
		return fail_msg("graphics: glfwCreateWindowSurface() failed!");

	log_i("graphics: Rendering surface initialized");

	return true;

}



bool init_device()
{

	static const int device_types[]={
		VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
		VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
	};

	GR_VKENUM_DEVICES(devices, gfx.vk.instance);

	if (devices_num < 1) {

		log_c("graphics: No Vulkan capable devices detected!");
		return false;

	}

	gfx.vk.device = NULL;

	for (int m=0; m < countof(device_types); m++)
		for (int n=0; n < devices_num; n++) {

			VkPhysicalDevice           dev = devices[n];
			VkPhysicalDeviceProperties properties;
			VkPhysicalDeviceFeatures   features;

			vkGetPhysicalDeviceProperties(dev, &properties);
			vkGetPhysicalDeviceFeatures(  dev, &features);

			if (properties.deviceType != device_types[m])
				continue;

			GR_VKENUM_QUEUE_FAMILIES(queues, dev);

			int graphics_index     = -1;
			int compute_index      = -1;
			int transfer_index     = -1;
			int presentation_index = -1;

			for (int k=0; k < queues_num; k++) {

				VkBool32 can_present = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(dev, k, gfx.vk.surface, &can_present);

				if ((queues[k].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (graphics_index < 0 || (queues[graphics_index].queueCount < queues[k].queueCount)))
					graphics_index = k;

				if ((queues[k].queueFlags & VK_QUEUE_COMPUTE_BIT) && (compute_index < 0 || (queues[compute_index].queueCount < queues[k].queueCount)))
					compute_index = k;

				if ((queues[k].queueFlags & VK_QUEUE_TRANSFER_BIT) && (transfer_index < 0 || (queues[transfer_index].queueCount < queues[k].queueCount)))
					transfer_index = k;

				if (can_present && (presentation_index < 0 || (queues[presentation_index].queueCount < queues[k].queueCount)))
					presentation_index = k;

			}

			bool has_swapchain    = false;
			bool has_glsl_shaders = false;

			GR_VKENUM_DEVICE_EXT(devexts, dev, NULL);

			for (int k=0; k < devexts_num; k++)
				if      (!strcmp(devexts[k].extensionName, "VK_KHR_swapchain"))  has_swapchain    = true;
				else if (!strcmp(devexts[k].extensionName, "VK_NV_glsl_shader")) has_glsl_shaders = true;

			if (!has_swapchain)
				continue;

			gfx.vk.device = dev;

			gfx.vk.graphics_queue_index = graphics_index;
			gfx.vk.graphics_queue_count = queues[graphics_index].queueCount;

			gfx.vk.compute_queue_index = compute_index;
			gfx.vk.compute_queue_count = queues[compute_index].queueCount;

			gfx.vk.transfer_queue_index = transfer_index;
			gfx.vk.transfer_queue_count = queues[transfer_index].queueCount;

			gfx.vk.presentation_queue_index = presentation_index;
			gfx.vk.presentation_queue_count = queues[presentation_index].queueCount;

			goto device_found;

		}

	log_c("graphics: No suitable GPU detected!");
	return false;

device_found:
	vkGetPhysicalDeviceProperties(gfx.vk.device, &gfx.vk.device_properties);
	vkGetPhysicalDeviceFeatures(  gfx.vk.device, &gfx.vk.device_features);

	vkGetPhysicalDeviceMemoryProperties(gfx.vk.device, &gfx.vk.memory_properties);

	log_i("graphics: Using device %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x %s",
			gfx.vk.device_properties.pipelineCacheUUID[0],  gfx.vk.device_properties.pipelineCacheUUID[1],
			gfx.vk.device_properties.pipelineCacheUUID[2],  gfx.vk.device_properties.pipelineCacheUUID[3],
			gfx.vk.device_properties.pipelineCacheUUID[4],  gfx.vk.device_properties.pipelineCacheUUID[5],
			gfx.vk.device_properties.pipelineCacheUUID[6],  gfx.vk.device_properties.pipelineCacheUUID[7],
			gfx.vk.device_properties.pipelineCacheUUID[8],  gfx.vk.device_properties.pipelineCacheUUID[9],
			gfx.vk.device_properties.pipelineCacheUUID[10], gfx.vk.device_properties.pipelineCacheUUID[11],
			gfx.vk.device_properties.pipelineCacheUUID[12], gfx.vk.device_properties.pipelineCacheUUID[13],
			gfx.vk.device_properties.pipelineCacheUUID[14], gfx.vk.device_properties.pipelineCacheUUID[15],
			gfx.vk.device_properties.deviceName);

//
// Init device
	VkDeviceQueueCreateInfo  dqci[2] = {};
	VkDeviceCreateInfo       dci = {};

	dqci[0].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	dqci[0].queueFamilyIndex = gfx.vk.graphics_queue_index;
	dqci[0].queueCount       = 1;
	dqci[0].pQueuePriorities = &(const float){ 1.0f };

	dqci[1].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	dqci[1].queueFamilyIndex = gfx.vk.presentation_queue_index;
	dqci[1].queueCount       = 1;
	dqci[1].pQueuePriorities = &(const float){ 1.0f };

	dci.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dci.pQueueCreateInfos       = dqci;
	dci.queueCreateInfoCount    = countof(dqci);
	dci.pEnabledFeatures        = &gfx.vk.device_features;
	dci.enabledExtensionCount   = gfx.ext.device_num;
	dci.ppEnabledExtensionNames = gfx.ext.device;
//	dci.ppEnabledLayerNames     = layer_names;
//	dci.enabledLayerCount       = sizeof(layer_names) / sizeof(layer_names[0]);

	if (vkCreateDevice(gfx.vk.device, &dci, NULL, &gfx.vk.gpu) != VK_SUCCESS)
		return fail_msg("graphics: vkCreateDevice() failed!");

	watch("%d", gfx.vk.graphics_queue_index);
	watch("%d", gfx.vk.compute_queue_index);
	watch("%d", gfx.vk.transfer_queue_index);
	watch("%d", gfx.vk.presentation_queue_index);

	vkGetDeviceQueue(gfx.vk.gpu, gfx.vk.graphics_queue_index,     0, &gfx.vk.graphics_queue);
	vkGetDeviceQueue(gfx.vk.gpu, gfx.vk.transfer_queue_index,     0, &gfx.vk.transfer_queue);
	vkGetDeviceQueue(gfx.vk.gpu, gfx.vk.presentation_queue_index, 0, &gfx.vk.presentation_queue);

	log_i("graphics: Rendering device ready");

	return true;

}



bool init_swapchain()
{

	VkSurfaceCapabilitiesKHR caps;
	VkSurfaceFormatKHR       format;
	VkPresentModeKHR         mode;
	VkExtent2D               extent;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gfx.vk.device, gfx.vk.surface, &caps);

	format.format     = VK_FORMAT_B8G8R8A8_UNORM;
	format.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	mode              = VK_PRESENT_MODE_FIFO_KHR;


// Here we should go through all formats
// and find one with highest number of bits that is non-linear
// That requires pixelformat descriptors which do not yet exist.
	GR_VKENUM_SURFACE_FORMATS(formats, gfx.vk.device, gfx.vk.surface);

	format = formats[0];


// vsync_enabled vsync_adaptive vsync-modes
//      F             X         IMMEDIATE FIFO-RELAXED FIFO
//      T             F         FIFO
//      T             T         FIFO-RELAXED FIFO

	bool has_immediate = false;
	bool has_adaptive  = false;

	GR_VKENUM_PRESENT_MODES(modes, gfx.vk.device, gfx.vk.surface);

	for (int n=0; n < modes_num; n++) {

		if (modes[n] == VK_PRESENT_MODE_IMMEDIATE_KHR)    has_immediate = true;
		if (modes[n] == VK_PRESENT_MODE_FIFO_RELAXED_KHR) has_adaptive  = true;

	}

	if (!*gfx.var.vsync_enable)
		mode =  has_immediate? VK_PRESENT_MODE_IMMEDIATE_KHR:
			has_adaptive?  VK_PRESENT_MODE_FIFO_RELAXED_KHR: VK_PRESENT_MODE_FIFO_KHR;

	else if (*gfx.var.vsync_adaptive)
		mode = has_adaptive? VK_PRESENT_MODE_FIFO_RELAXED_KHR: VK_PRESENT_MODE_FIFO_KHR;

	int w, h;
	glfwGetWindowSize(gfx.window, &w, &h);

	extent.width  = clampu(w, caps.minImageExtent.width,  caps.maxImageExtent.width);
	extent.height = clampu(h, caps.minImageExtent.height, caps.maxImageExtent.height);

	gfx.vk.swapchain_length = *gfx.var.triple_buffer? 3: 2;

	if (gfx.vk.swapchain_length > caps.maxImageCount)
		gfx.vk.swapchain_length = caps.maxImageCount;

	VkSwapchainCreateInfoKHR scci = {};

	scci.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	scci.surface          = gfx.vk.surface;
	scci.minImageCount    = gfx.vk.swapchain_length;
	scci.imageFormat      = format.format;
	scci.imageColorSpace  = format.colorSpace;
	scci.imageExtent      = extent;
	scci.imageArrayLayers = 1;
	scci.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	const uint queue_indices[2] = {
		(uint)gfx.vk.graphics_queue_index,
		(uint)gfx.vk.presentation_queue_index
	};

	if (gfx.vk.graphics_queue_index != gfx.vk.presentation_queue_index) {

		scci.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
		scci.queueFamilyIndexCount = 2;
		scci.pQueueFamilyIndices   = queue_indices;

	} else
		scci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

	scci.preTransform   = caps.currentTransform;
	scci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	scci.presentMode    = mode;
	scci.clipped        = VK_TRUE;
	scci.oldSwapchain   = gfx.vk.swapchain;

	if (vkCreateSwapchainKHR(gfx.vk.gpu, &scci, NULL, &gfx.vk.swapchain) != VK_SUCCESS)
		return fail_msg("graphics: vkCreateSwapchainKHR() failed!");

	log_i("graphics: Swapchain initialized");

	gfx.vk.swapchain_width  = extent.width;
	gfx.vk.swapchain_height = extent.height;
	gfx.vk.swapchain_format = format.format;

	vkGetSwapchainImagesKHR(gfx.vk.gpu, gfx.vk.swapchain, (uint*)&gfx.vk.swapchain_length, gfx.vk.swapchain_images);


	for (int n=0; n < gfx.vk.swapchain_length; n++) {

		VkImageViewCreateInfo ivci = {};

		ivci.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ivci.image                           = gfx.vk.swapchain_images[n];
		ivci.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
		ivci.format                          = gfx.vk.swapchain_format;
		ivci.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		ivci.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		ivci.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		ivci.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		ivci.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		ivci.subresourceRange.baseMipLevel   = 0;
		ivci.subresourceRange.levelCount     = 1;
		ivci.subresourceRange.baseArrayLayer = 0;
		ivci.subresourceRange.layerCount     = 1;

		if (vkCreateImageView(gfx.vk.gpu, &ivci, NULL, &gfx.vk.swapchain_views[n]) != VK_SUCCESS)
			return fail_msg("graphics: vkCreateImageView() failed!");

	}

	log_i("graphics: Swapchain views ready");
	return true;

}



bool init_commandpool()
{

	VkCommandPoolCreateInfo     cpci = {};
	VkCommandBufferAllocateInfo cbai = {};

	// Create Graphics Pool
	{
		cpci.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cpci.queueFamilyIndex = gfx.vk.graphics_queue_index;
		cpci.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(gfx.vk.gpu, &cpci, NULL, &gfx.vk.command_pool) != VK_SUCCESS)
			return fail_msg("graphics: [graphics] vkCreateCommandPool() failed!");

		cbai.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cbai.commandPool        = gfx.vk.command_pool;
		cbai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cbai.commandBufferCount = gfx.vk.swapchain_length;

		if (vkAllocateCommandBuffers(gfx.vk.gpu, &cbai, &gfx.vk.command_buffer[0]) != VK_SUCCESS)
			return fail_msg("graphics: [graphics] vkAllocateCommandBuffers() failed!");
	}

	// Create Transfer Pool
	{
		cpci.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cpci.queueFamilyIndex = gfx.vk.transfer_queue_index;
		cpci.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(gfx.vk.gpu, &cpci, NULL, &gfx.vk.transfer_pool) != VK_SUCCESS)
			return fail_msg("graphics: [transfer] vkCreateCommandPool() failed!");

		cbai.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cbai.commandPool        = gfx.vk.transfer_pool;
		cbai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cbai.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(gfx.vk.gpu, &cbai, &gfx.vk.transfer_buffer) != VK_SUCCESS)
			return fail_msg("graphics: [transfer] vkAllocateCommandBuffers() failed!");
	}

	log_i("graphics: Command queues ready");

	return true;

}



bool init_synchronization()
{

	VkSemaphoreCreateInfo sci = {};
	VkFenceCreateInfo     fci = {};

	sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateSemaphore(gfx.vk.gpu, &sci, NULL, &gfx.vk.signal_image_ready) != VK_SUCCESS)
		return fail_msg("graphics: [image] vkCreateSemaphore() failed!");

	if (vkCreateSemaphore(gfx.vk.gpu, &sci, NULL, &gfx.vk.signal_render_complete) != VK_SUCCESS)
		return fail_msg("graphics: [render] vkCreateSemaphore() failed!");

	for (int n=0; n < gfx.vk.swapchain_length; n++)
		if (vkCreateFence(gfx.vk.gpu, &fci, NULL, &gfx.vk.command_fence[n]))
			return fail_msg("graphics: [commandbuffer] vkCreateFence() failed!");

	return true;

}



bool init_descriptors()
{

	{

		VkDescriptorPoolSize            dps   = {};
		VkDescriptorPoolCreateInfo      dpci  = {};
		VkDescriptorSetLayoutBinding    dslb  = {};
		VkDescriptorSetLayoutCreateInfo dslci = {};

		dps.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		dps.descriptorCount = 256;

		dpci.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		dpci.poolSizeCount = 1;
		dpci.pPoolSizes    = &dps;
		dpci.maxSets       = 256;

		if (vkCreateDescriptorPool(gfx.vk.gpu, &dpci, NULL, &gfx.vk.descriptor_uniform_pool) != VK_SUCCESS)
			return fail_msg("graphics: [uniform] vkCreateDescriptorPool() failed!");

		dslb.binding            = 0;
		dslb.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		dslb.descriptorCount    = 1;
		dslb.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		dslb.pImmutableSamplers = NULL;

		dslci.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		dslci.bindingCount = 1;
		dslci.pBindings    = &dslb;

		if (vkCreateDescriptorSetLayout(gfx.vk.gpu, &dslci, NULL, &gfx.vk.descriptor_uniform_layout) != VK_SUCCESS)
			return fail_msg("graphics: [uniform] vkCreateDescriptorSetLayout() failed!");

	}

	{
		VkDescriptorPoolSize            dps     = {};
		VkDescriptorPoolCreateInfo      dpci    = {};
		VkDescriptorSetLayoutBinding    dslb[3] = {};
		VkDescriptorSetLayoutCreateInfo dslci   = {};

		dps.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		dps.descriptorCount = GPU_TEXTURES_NUM;

		dpci.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		dpci.poolSizeCount = 1;
		dpci.pPoolSizes    = &dps;
		dpci.maxSets       = 4;

		if (vkCreateDescriptorPool(gfx.vk.gpu, &dpci, NULL, &gfx.vk.descriptor_texture_pool) != VK_SUCCESS)
			return fail_msg("graphics: [texture] vkCreateDescriptorPool() failed!");

		dslb[0].binding            = GPU_TEXTURE_2D_BINDING;
		dslb[0].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		dslb[0].descriptorCount    = GPU_TEXTURES_2D;
		dslb[0].stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		dslb[0].pImmutableSamplers = NULL;

		dslb[1].binding            = GPU_TEXTURE_3D_BINDING;
		dslb[1].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		dslb[1].descriptorCount    = GPU_TEXTURES_3D;
		dslb[1].stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		dslb[1].pImmutableSamplers = NULL;

		dslb[2].binding            = GPU_TEXTURE_CUBE_BINDING;
		dslb[2].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		dslb[2].descriptorCount    = GPU_TEXTURES_CUBE;
		dslb[2].stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		dslb[2].pImmutableSamplers = NULL;

		dslci.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		dslci.bindingCount = countof(dslb);
		dslci.pBindings    = &dslb[0];

		if (vkCreateDescriptorSetLayout(gfx.vk.gpu, &dslci, NULL, &gfx.vk.descriptor_texture_layout) != VK_SUCCESS)
			return fail_msg("graphics: [texture] vkCreateDescriptorSetLayout() failed!");

	}

	const VkDescriptorSetLayout descriptors[] = {
		//gfx.vk.descriptor_uniform_layout,
		gfx.vk.descriptor_texture_layout,
		gfx.vk.descriptor_uniform_layout,
		gfx.vk.descriptor_uniform_layout
	};

	VkPipelineLayoutCreateInfo plci = {};

	plci.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	plci.setLayoutCount         = countof(descriptors);
	plci.pSetLayouts            = &descriptors[0];
	plci.pushConstantRangeCount = 0;
	plci.pPushConstantRanges    = 0;

	VkPipelineLayout layout = NULL;

	if (vkCreatePipelineLayout(gfx.vk.gpu, &plci, NULL, &gfx.vk.pipeline_layout) != VK_SUCCESS)
		return false;

	return true;

}



VKAPI_ATTR VkBool32 VKAPI_CALL debug(
		VkDebugReportFlagsEXT      flags,
		VkDebugReportObjectTypeEXT obj_type,
		uint64_t                   obj,
		size_t                     location,
		int32_t                    code,
		const char                 *layer_prefix,
		const char                 *msg,
		void                       *ptr)
{

	log_i("graphics: vulkan(lyr=%s, loc=%ld, cod=%d, obj=%d:%ld): %s",
		layer_prefix, location, code, obj_type, obj, msg);

	return VK_FALSE;

}

