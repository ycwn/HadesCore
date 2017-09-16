

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/list.h"
#include "core/logger.h"
#include "core/string.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/vertexformat.h"
#include "gr/framebuffer.h"
#include "gr/rendertarget.h"
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
GR_VKSYM_DEF(vkCmdBindPipeline);
GR_VKSYM_DEF(vkCmdBindIndexBuffer);
GR_VKSYM_DEF(vkCmdBindVertexBuffers);
GR_VKSYM_DEF(vkCmdCopyBuffer);
GR_VKSYM_DEF(vkCmdCopyBufferToImage);
GR_VKSYM_DEF(vkCmdDraw);
GR_VKSYM_DEF(vkCmdDrawIndexed);
GR_VKSYM_DEF(vkCmdEndRenderPass);
GR_VKSYM_DEF(vkCmdPipelineBarrier);
GR_VKSYM_DEF(vkCreateBuffer);
GR_VKSYM_DEF(vkCreateCommandPool);
GR_VKSYM_DEF(vkCreateDescriptorSetLayout);
GR_VKSYM_DEF(vkCreateDescriptorPool);
GR_VKSYM_DEF(vkCreateDevice);
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
GR_VKSYM_DEF(vkDestroyDescriptorSetLayout);
GR_VKSYM_DEF(vkDestroyDescriptorPool);
GR_VKSYM_DEF(vkDestroyDevice);
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
GR_VKSYM_DEF(vkGetDeviceQueue);
GR_VKSYM_DEF(vkGetBufferMemoryRequirements);
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
GR_VKSYM_DEF(vkUnmapMemory);
GR_VKSYM_DEF(vkUpdateDescriptorSets);

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
	T    name[(N)];                                   \
	do {                                              \
		func(__VA_ARGS__, &name##_num, NULL);     \
		watch("%d", name##_num);                  \
		if (name##_num > (N)) name##_num = (N);   \
		func(__VA_ARGS__, &name##_num, &name[0]); \
	} while (0)

#define GR_VKENUM_SDL_EXTENSIONS(name, wnd) \
	GR_VKENUMERATE(name, 16, const char*, SDL_Vulkan_GetInstanceExtensions, (wnd));

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
//	"VK_LAYER_LUNARG_standard_validation"
//	"VK_LAYER_GOOGLE_unique_objects",
	"VK_LAYER_LUNARG_device_limits",
	"VK_LAYER_LUNARG_core_validation",
	"VK_LAYER_LUNARG_image",
	"VK_LAYER_LUNARG_object_tracker",
	"VK_LAYER_LUNARG_parameter_validation",
	"VK_LAYER_LUNARG_swapchain",
	"VK_LAYER_GOOGLE_threading"
};

static const char *validation_extensions[]={
	"VK_EXT_debug_report"
};


static inline bool fail_msg(const char *msg) { log_d(msg); return false; }

static void reset(graphics *gr);
static void destroy(graphics *gr);
static bool init_vulkan(graphics *gr);
static bool init_device(graphics *gr);
static bool init_swapchain(graphics *gr);
static bool init_pipeline(graphics *gr);
static bool init_commandpool(graphics *gr);
static bool init_synchronization(graphics *gr);
static bool init_descriptors(graphics *gr);

static VKAPI_ATTR VkBool32 VKAPI_CALL debug(
		VkDebugReportFlagsEXT      flags,
		VkDebugReportObjectTypeEXT obj_type,
		uint64_t                   obj,
		size_t                     location,
		int32_t                    code,
		const char                 *layer_prefix,
		const char                 *msg,
		void                       *ptr);




graphics *gr_create()
{

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
		return NULL;

	SDL_setenv("SDL_X11_XCB_LIBRARY", "libX11-xcb.so.1", 0);

	if (SDL_Vulkan_LoadLibrary(NULL) < 0)
		return NULL;

	graphics *gr = malloc(sizeof(graphics));

	gr->var.screen_width      = var_new("gr.screen.width",   "1024");
	gr->var.screen_height     = var_new("gr.screen.height",   "576");
	gr->var.screen_fullscreen = var_new("gr.screen.full",       "0");
	gr->var.vsync_enable      = var_new("gr.vsync.enable",      "1");
	gr->var.vsync_adaptive    = var_new("gr.vsync.adaptive",    "1");
	gr->var.triple_buffer     = var_new("gr.tribuffer",         "0");
	gr->var.validate          = var_new("gr.validate",          "0");

	gr->ext.instance_num = 0;
	gr->ext.device_num   = 0;

	mzero(gr->ext.instance);
	mzero(gr->ext.device);

	reset(gr);

	gr_rendertarget_create(gr);
	gr_framebuffer_create(gr);
	gr_shader_create(gr);
	gr_command_create();
	gr_commandqueue_create();

	return gr;

}



void gr_destroy(graphics *gr)
{

	if (gr == NULL)
		return;

	gr_commandqueue_destroy();
	gr_command_destroy();
	gr_shader_destroy();
	gr_framebuffer_destroy();
	gr_rendertarget_destroy();

	destroy(gr);

	var_del(gr->var.screen_width);
	var_del(gr->var.screen_height);
	var_del(gr->var.screen_fullscreen);
	var_del(gr->var.vsync_enable);
	var_del(gr->var.vsync_adaptive);
	var_del(gr->var.triple_buffer);
	var_del(gr->var.validate);

	SDL_Vulkan_UnloadLibrary();
	SDL_QuitSubSystem(SDL_INIT_VIDEO);

}



bool gr_request_instance_extension(graphics *gr, const char *ext)
{

	for (int n=0; n < gr->ext.instance_num; n++)
		if (!strcmp(ext, gr->ext.instance[n]))
			return true;

	if (gr->ext.instance_num >= GR_INSTANCE_EXT_MAX)
		return false;

	gr->ext.instance[gr->ext.instance_num++] = ext;

	return true;

}



bool gr_request_device_extension(graphics *gr, const char *ext)
{

	for (int n=0; n < gr->ext.device_num; n++)
		if (!strcmp(ext, gr->ext.device[n]))
			return true;

	if (gr->ext.device_num >= GR_DEVICE_EXT_MAX)
		return false;

	gr->ext.device[gr->ext.device_num++] = ext;

	return true;

}

static gr_command triangle;

bool gr_set_video(graphics *gr)
{

	destroy(gr);

	uint flags = SDL_WINDOW_VULKAN;

	if (gr->var.screen_fullscreen->integer)
		flags |= SDL_WINDOW_FULLSCREEN;

	gr->window = SDL_CreateWindow("Hades Core",
		SDL_WINDOWPOS_CENTERED,        SDL_WINDOWPOS_CENTERED,
		gr->var.screen_width->integer, gr->var.screen_height->integer, flags);

	watch("%p", gr->window);

	if (gr->window == NULL ||
		!init_vulkan(gr)      || !init_device(gr)          || !init_swapchain(gr)   ||
		!init_commandpool(gr) || !init_synchronization(gr) || !init_descriptors(gr) ||

		!gr_framebuffer_init()) {

		log_e("graphics: Graphics initialization sequence failed");
		destroy(gr);

		return false;

	}

	gr_command_init(&triangle, 1);
	triangle.shader = gr_shader_load("shaders/triangle.a");
	triangle.count  = 3;

	log_i("graphics: Graphics initialization sequence complete");

	return true;

}



void gr_submit(graphics *gr)
{

	vkAcquireNextImageKHR(
		gr->vk.gpu,
		gr->vk.swapchain, 1000000000,
		gr->vk.signal_image_ready, NULL, (uint*)&gr->vk.swapchain_curr);

	gr_commandqueue_enqueue(&triangle, 1);
	gr_commandqueue_consume();
	gr_framebuffer_select();

	VkCommandBuffer  curr_cmd    = gr->vk.command_buffer[gr->vk.swapchain_curr];
	gr_rendertarget *curr_target = NULL;
	gr_shader       *curr_shader = NULL;
	//vbo             *curr_vbo    = NULL;
	//ubo             *curr_ubo    = NULL;
	//core::surface   *curr_tex    = NULL;

	VkCommandBufferBeginInfo cbbi;

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

			VkRenderPassBeginInfo rpi;

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
//			curr_vbo    = NULL;
//			curr_ubo    = NULL;
//			curr_tex    = NULL;

			vkCmdBindPipeline(curr_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, curr_shader->pipeline);

		}
/*
		if (curr_vbo != cmd->v) {

			curr_vbo = cmd->v;

			if (curr_vbo->get_vertex_buffer() != NULL)
				vkCmdBindVertexBuffers(
					curr_cmd,
					0, 1,
					&(const VkBuffer){ curr_vbo->get_vertex_buffer() },
					&(const VkDeviceSize){ 0 });

			if (curr_vbo->get_index_buffer() != NULL)
				vkCmdBindIndexBuffer(
					curr_cmd,
					curr_vbo->get_index_buffer(),
					0,
					VK_INDEX_TYPE_UINT16);

		}
*/
/*		if (curr_ubo != cmd->u || curr_tex != cmd->t) {

			curr_ubo = cmd->u;
			curr_tex = cmd->t;

			const VkDescriptorSet descriptors[]={
				curr_ubo->get_descriptor(),
				curr_tex->get_descriptor()
			};

			vkCmdBindDescriptorSets(
				curr_cmd,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				curr_shader->get_layout(),
				0, countof(descriptors), &descriptors[0],
				0, NULL);

		}

		if (curr_vbo->has_indices())
			vkCmdDrawIndexed(curr_cmd, cmd->count, 1, 0, 0, 0);

		else*/
			vkCmdDraw(curr_cmd, cmd->count, 1, 0, 0);

	}

	if (curr_target != NULL)
		vkCmdEndRenderPass(curr_cmd);

	if (vkEndCommandBuffer(curr_cmd) != VK_SUCCESS)
		return;

	VkSubmitInfo     si;
	VkPresentInfoKHR pi;

	si.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	si.waitSemaphoreCount   = 1;
	si.pWaitSemaphores      = &gr->vk.signal_image_ready;
	si.pWaitDstStageMask    = &(const VkPipelineStageFlags){ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	si.commandBufferCount   = 1;
	si.pCommandBuffers      = &gr->vk.command_buffer[gr->vk.swapchain_curr];
	si.signalSemaphoreCount = 1;
	si.pSignalSemaphores    = &gr->vk.signal_render_complete;

	vkQueueSubmit(gr->vk.graphics_queue, 1, &si, NULL);

	pi.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	pi.waitSemaphoreCount = 1;
	pi.pWaitSemaphores    = &gr->vk.signal_render_complete;
	pi.swapchainCount     = 1;
	pi.pSwapchains        = &gr->vk.swapchain;
	pi.pImageIndices      = (uint*)&gr->vk.swapchain_curr;
	pi.pResults           = NULL;

	vkQueuePresentKHR(gr->vk.presentation_queue, &pi);

}



void reset(graphics *gr)
{

	gr->window = NULL;

	gr->vk.instance = NULL;
	gr->vk.device   = NULL;
	gr->vk.gpu      = NULL;

 	memset(&gr->vk.memory_properties, 0, sizeof(gr->vk.memory_properties));

	gr->vk.graphics_queue_index = -1;
	gr->vk.graphics_queue_count =  0;
	gr->vk.graphics_queue       = NULL;

	gr->vk.compute_queue_index = -1;
	gr->vk.compute_queue_count =  0;
	gr->vk.compute_queue       = NULL;

	gr->vk.transfer_queue_index = -1;
	gr->vk.transfer_queue_count =  0;
	gr->vk.transfer_queue       = NULL;

	gr->vk.presentation_queue_index = -1;
	gr->vk.presentation_queue_count =  0;
	gr->vk.presentation_queue       = NULL;

	gr->vk.surface = NULL;

	gr->vk.swapchain_width  = 0;
	gr->vk.swapchain_height = 0;
	gr->vk.swapchain_length = 0;
	gr->vk.swapchain_curr   = -1;
	gr->vk.swapchain_format = VK_FORMAT_UNDEFINED;
	gr->vk.swapchain        = NULL;

	gr->vk.command_pool = NULL;

	mzero(gr->vk.command_buffer);

	gr->vk.transfer_pool   = NULL;
	gr->vk.transfer_buffer = NULL;

	gr->vk.signal_image_ready     = NULL;
	gr->vk.signal_render_complete = NULL;

	gr->vk.descriptor_uniform_pool = NULL;
	gr->vk.descriptor_texture_pool = NULL;

	gr->vk.descriptor_uniform_layout = NULL;
	gr->vk.descriptor_texture_layout = NULL;

	gr->vk.pipeline_layout = NULL;

}



void destroy(graphics *gr)
{

	if (gr->vk.pipeline_layout != NULL)
		vkDestroyPipelineLayout(gr->vk.gpu, gr->vk.pipeline_layout, NULL);

	if (gr->vk.descriptor_uniform_layout != NULL)
		vkDestroyDescriptorSetLayout(gr->vk.gpu, gr->vk.descriptor_uniform_layout, NULL);

	if (gr->vk.descriptor_texture_layout != NULL)
		vkDestroyDescriptorSetLayout(gr->vk.gpu, gr->vk.descriptor_texture_layout, NULL);

	if (gr->vk.descriptor_uniform_pool != NULL)
		vkDestroyDescriptorPool(gr->vk.gpu, gr->vk.descriptor_uniform_pool, NULL);

	if (gr->vk.descriptor_texture_pool != NULL)
		vkDestroyDescriptorPool(gr->vk.gpu, gr->vk.descriptor_texture_pool, NULL);

	if (gr->vk.signal_render_complete != NULL)
		vkDestroySemaphore(gr->vk.gpu, gr->vk.signal_render_complete, NULL);

	if (gr->vk.signal_image_ready != NULL)
		vkDestroySemaphore(gr->vk.gpu, gr->vk.signal_image_ready, NULL);

	if (gr->vk.command_pool != NULL)
		vkDestroyCommandPool(gr->vk.gpu, gr->vk.command_pool, NULL);

	if (gr->vk.transfer_pool != NULL)
		vkDestroyCommandPool(gr->vk.gpu, gr->vk.transfer_pool, NULL);

	if (gr->vk.swapchain != NULL) {

		for (int n=0; n < gr->vk.swapchain_length; n++)
			vkDestroyImageView(gr->vk.gpu, gr->vk.swapchain_views[n], NULL);

		vkDestroySwapchainKHR(gr->vk.gpu, gr->vk.swapchain, NULL);

	}

	if (gr->vk.gpu != NULL)
		vkDestroyDevice(gr->vk.gpu, NULL);

	if (gr->vk.surface != NULL)
		vkDestroySurfaceKHR(gr->vk.instance, gr->vk.surface, NULL);

//	if (callback != NULL)
//		vkDestroyDebugReportCallbackEXT(instance, callback, NULL);

	if (gr->vk.instance != NULL)
		vkDestroyInstance(gr->vk.instance, NULL);

	if (gr->window != NULL)
		SDL_DestroyWindow(gr->window);

	reset(gr);

}



bool init_vulkan(graphics *gr)
{

	vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr();

	if (vkGetInstanceProcAddr == NULL)
		return fail_msg("graphics: SDL_Vulkan_GetVkGetInstanceProcAddr() failed!");

	GR_VKSYM(vkCreateInstance);
	GR_VKSYM(vkEnumerateInstanceExtensionProperties);

	GR_VKENUM_SDL_EXTENSIONS(sdl_ext, gr->window);

	if (sdl_ext_num == 0)
		return fail_msg("graphics: SDL_Vulkan_GetInstanceExtensions() failed!");

	for (int n=0; n < sdl_ext_num; n++)
		gr_request_instance_extension(gr, sdl_ext[n]);

	if (gr->var.validate->integer)
		for (int n=0; n < countof(validation_extensions); n++)
			gr_request_instance_extension(gr, validation_extensions[n]);

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
	vkici.enabledExtensionCount   = gr->ext.instance_num;
	vkici.ppEnabledExtensionNames = gr->ext.instance;
	vkici.enabledLayerCount       = gr->var.validate->integer? countof(validation_layers): 0;
	vkici.ppEnabledLayerNames     = gr->var.validate->integer? validation_layers: NULL;

	if (vkCreateInstance(&vkici, NULL, &gr->vk.instance) != VK_SUCCESS)
		return fail_msg("graphics: vkCreateInstance() failed!");

	log_i("graphics: Vulkan initialized");

	GR_VKSYM(vkAllocateCommandBuffers);
	GR_VKSYM(vkAllocateDescriptorSets);
	GR_VKSYM(vkAllocateMemory);
	GR_VKSYM(vkBeginCommandBuffer);
	GR_VKSYM(vkBindBufferMemory);
	GR_VKSYM(vkBindImageMemory);
	GR_VKSYM(vkCmdBeginRenderPass);
	GR_VKSYM(vkCmdBindDescriptorSets);
	GR_VKSYM(vkCmdBindPipeline);
	GR_VKSYM(vkCmdBindIndexBuffer);
	GR_VKSYM(vkCmdBindVertexBuffers);
	GR_VKSYM(vkCmdCopyBuffer);
	GR_VKSYM(vkCmdCopyBufferToImage);
	GR_VKSYM(vkCmdDraw);
	GR_VKSYM(vkCmdDrawIndexed);
	GR_VKSYM(vkCmdEndRenderPass);
	GR_VKSYM(vkCmdPipelineBarrier);
	GR_VKSYM(vkCreateBuffer);
	GR_VKSYM(vkCreateCommandPool);
	GR_VKSYM(vkCreateDescriptorSetLayout);
	GR_VKSYM(vkCreateDescriptorPool);
	GR_VKSYM(vkCreateDevice);
	GR_VKSYM(vkCreateFramebuffer);
	GR_VKSYM(vkCreateGraphicsPipelines);
	GR_VKSYM(vkCreateImage);
	GR_VKSYM(vkCreateImageView);
	GR_VKSYM(vkCreatePipelineLayout);
	GR_VKSYM(vkCreateRenderPass);
	GR_VKSYM(vkCreateSampler);
	GR_VKSYM(vkCreateSemaphore);
	GR_VKSYM(vkCreateShaderModule);
	GR_VKSYM(vkDestroyBuffer);
	GR_VKSYM(vkDestroyCommandPool);
	GR_VKSYM(vkDestroyDescriptorSetLayout);
	GR_VKSYM(vkDestroyDescriptorPool);
	GR_VKSYM(vkDestroyDevice);
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
	GR_VKSYM(vkEnumeratePhysicalDevices);
	GR_VKSYM(vkFreeMemory);
	GR_VKSYM(vkGetDeviceQueue);
	GR_VKSYM(vkGetBufferMemoryRequirements);
	GR_VKSYM(vkGetImageMemoryRequirements);
	GR_VKSYM(vkGetPhysicalDeviceFeatures);
	GR_VKSYM(vkGetPhysicalDeviceFormatProperties);
	GR_VKSYM(vkGetPhysicalDeviceMemoryProperties);
	GR_VKSYM(vkGetPhysicalDeviceProperties);
	GR_VKSYM(vkGetPhysicalDeviceQueueFamilyProperties);
	GR_VKSYM(vkMapMemory);
	GR_VKSYM(vkQueueSubmit);
	GR_VKSYM(vkQueueWaitIdle);
	GR_VKSYM(vkUnmapMemory);
	GR_VKSYM(vkUpdateDescriptorSets);

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

	if (gr->var.validate->integer) {

		VkDebugReportCallbackCreateInfoEXT vkdrcci = {};

		vkdrcci.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		vkdrcci.flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		vkdrcci.pfnCallback = debug;

		if (vkCreateDebugReportCallbackEXT(gr->vk.instance, &vkdrcci, NULL, &gr->vk.callback) != VK_SUCCESS)
			log_e("graphics: warning: Failed to set up debug callback!");

	}

	if (!SDL_Vulkan_CreateSurface(gr->window, gr->vk.instance, &gr->vk.surface))
		return fail_msg("graphics: SDL_Vulkan_CreateSurface() failed!");

	log_i("graphics: Rendering surface initialized");

	return true;

}



bool init_device(graphics *gr)
{

	static const int device_types[]={
		VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
		VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
	};

	GR_VKENUM_DEVICES(devices, gr->vk.instance);

	if (devices_num < 1) {

		log_c("graphics: No Vulkan capable devices detected!");
		return false;

	}

	gr->vk.device = NULL;

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
				vkGetPhysicalDeviceSurfaceSupportKHR(dev, k, gr->vk.surface, &can_present);

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

			gr->vk.device = dev;

			gr->vk.graphics_queue_index = graphics_index;
			gr->vk.graphics_queue_count = queues[graphics_index].queueCount;

			gr->vk.compute_queue_index = compute_index;
			gr->vk.compute_queue_count = queues[compute_index].queueCount;

			gr->vk.transfer_queue_index = transfer_index;
			gr->vk.transfer_queue_count = queues[transfer_index].queueCount;

			gr->vk.presentation_queue_index = presentation_index;
			gr->vk.presentation_queue_count = queues[presentation_index].queueCount;

			goto device_found;

		}

	log_c("graphics: No suitable GPU detected!");
	return false;

device_found:
	vkGetPhysicalDeviceProperties(gr->vk.device, &gr->vk.device_properties);
	vkGetPhysicalDeviceFeatures(  gr->vk.device, &gr->vk.device_features);

	vkGetPhysicalDeviceMemoryProperties(gr->vk.device, &gr->vk.memory_properties);

	log_i("graphics: Using device %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x %s",
			gr->vk.device_properties.pipelineCacheUUID[0],  gr->vk.device_properties.pipelineCacheUUID[1],
			gr->vk.device_properties.pipelineCacheUUID[2],  gr->vk.device_properties.pipelineCacheUUID[3],
			gr->vk.device_properties.pipelineCacheUUID[4],  gr->vk.device_properties.pipelineCacheUUID[5],
			gr->vk.device_properties.pipelineCacheUUID[6],  gr->vk.device_properties.pipelineCacheUUID[7],
			gr->vk.device_properties.pipelineCacheUUID[8],  gr->vk.device_properties.pipelineCacheUUID[9],
			gr->vk.device_properties.pipelineCacheUUID[10], gr->vk.device_properties.pipelineCacheUUID[11],
			gr->vk.device_properties.pipelineCacheUUID[12], gr->vk.device_properties.pipelineCacheUUID[13],
			gr->vk.device_properties.pipelineCacheUUID[14], gr->vk.device_properties.pipelineCacheUUID[15],
			gr->vk.device_properties.deviceName);

//
// Init device
	VkDeviceQueueCreateInfo  dqci[2] = {};
	VkPhysicalDeviceFeatures pdf = {};
	VkDeviceCreateInfo       dci = {};

	dqci[0].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	dqci[0].queueFamilyIndex = gr->vk.graphics_queue_index;
	dqci[0].queueCount       = 1;
	dqci[0].pQueuePriorities = &(const float){ 1.0f };

	dqci[1].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	dqci[1].queueFamilyIndex = gr->vk.presentation_queue_index;
	dqci[1].queueCount       = 1;
	dqci[1].pQueuePriorities = &(const float){ 1.0f };

	dci.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dci.pQueueCreateInfos       = dqci;
	dci.queueCreateInfoCount    = countof(dqci);
	dci.pEnabledFeatures        = &pdf;
	dci.enabledExtensionCount   = gr->ext.device_num;
	dci.ppEnabledExtensionNames = gr->ext.device;
//	dci.ppEnabledLayerNames     = layer_names;
//	dci.enabledLayerCount       = sizeof(layer_names) / sizeof(layer_names[0]);

	if (vkCreateDevice(gr->vk.device, &dci, NULL, &gr->vk.gpu) != VK_SUCCESS)
		return fail_msg("graphics: vkCreateDevice() failed!");

	watch("%d", gr->vk.graphics_queue_index);
	watch("%d", gr->vk.compute_queue_index);
	watch("%d", gr->vk.transfer_queue_index);
	watch("%d", gr->vk.presentation_queue_index);

	vkGetDeviceQueue(gr->vk.gpu, gr->vk.graphics_queue_index,     0, &gr->vk.graphics_queue);
	vkGetDeviceQueue(gr->vk.gpu, gr->vk.transfer_queue_index,     0, &gr->vk.transfer_queue);
	vkGetDeviceQueue(gr->vk.gpu, gr->vk.presentation_queue_index, 0, &gr->vk.presentation_queue);

	log_i("graphics: Rendering device ready");

	return true;

}



bool init_swapchain(graphics *gr)
{

	VkSurfaceCapabilitiesKHR caps;
	VkSurfaceFormatKHR       format;
	VkPresentModeKHR         mode;
	VkExtent2D               extent;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gr->vk.device, gr->vk.surface, &caps);

	format.format     = VK_FORMAT_B8G8R8A8_UNORM;
	format.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	mode              = VK_PRESENT_MODE_FIFO_KHR;


// Here we should go through all formats
// and find one with highest number of bits that is non-linear
// That requires pixelformat descriptors which do not yet exist.
	GR_VKENUM_SURFACE_FORMATS(formats, gr->vk.device, gr->vk.surface);

	format = formats[0];


// vsync_enabled vsync_adaptive vsync-modes
//      F             X         IMMEDIATE FIFO-RELAXED FIFO
//      T             F         FIFO
//      T             T         FIFO-RELAXED FIFO

	bool has_immediate = false;
	bool has_adaptive  = false;

	GR_VKENUM_PRESENT_MODES(modes, gr->vk.device, gr->vk.surface);

	for (int n=0; n < modes_num; n++) {

		if (modes[n] == VK_PRESENT_MODE_IMMEDIATE_KHR)    has_immediate = true;
		if (modes[n] == VK_PRESENT_MODE_FIFO_RELAXED_KHR) has_adaptive  = true;

	}

	if (!gr->var.vsync_enable->integer)
		mode =  has_immediate? VK_PRESENT_MODE_IMMEDIATE_KHR:
			has_adaptive?  VK_PRESENT_MODE_FIFO_RELAXED_KHR: VK_PRESENT_MODE_FIFO_KHR;

	else if (gr->var.vsync_adaptive->integer)
		mode = has_adaptive? VK_PRESENT_MODE_FIFO_RELAXED_KHR: VK_PRESENT_MODE_FIFO_KHR;

	int w, h;
	SDL_GetWindowSize(gr->window, &w, &h);

	extent.width  = clampu(w, caps.minImageExtent.width,  caps.maxImageExtent.width);
	extent.height = clampu(h, caps.minImageExtent.height, caps.maxImageExtent.height);

	gr->vk.swapchain_length = gr->var.triple_buffer->integer? 3: 2;

	if (gr->vk.swapchain_length > caps.maxImageCount)
		gr->vk.swapchain_length = caps.maxImageCount;

	VkSwapchainCreateInfoKHR scci = {};

	scci.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	scci.surface          = gr->vk.surface;
	scci.minImageCount    = gr->vk.swapchain_length;
	scci.imageFormat      = format.format;
	scci.imageColorSpace  = format.colorSpace;
	scci.imageExtent      = extent;
	scci.imageArrayLayers = 1;
	scci.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	const uint queue_indices[2] = {
		(uint)gr->vk.graphics_queue_index,
		(uint)gr->vk.presentation_queue_index
	};

	if (gr->vk.graphics_queue_index != gr->vk.presentation_queue_index) {

		scci.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
		scci.queueFamilyIndexCount = 2;
		scci.pQueueFamilyIndices   = queue_indices;

	} else
		scci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

	scci.preTransform   = caps.currentTransform;
	scci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	scci.presentMode    = mode;
	scci.clipped        = VK_TRUE;
	scci.oldSwapchain   = gr->vk.swapchain;

	if (vkCreateSwapchainKHR(gr->vk.gpu, &scci, NULL, &gr->vk.swapchain) != VK_SUCCESS)
		return fail_msg("graphics: vkCreateSwapchainKHR() failed!");

	log_i("graphics: Swapchain initialized");

	gr->vk.swapchain_width  = extent.width;
	gr->vk.swapchain_height = extent.height;
	gr->vk.swapchain_format = format.format;

	vkGetSwapchainImagesKHR(gr->vk.gpu, gr->vk.swapchain, (uint*)&gr->vk.swapchain_length, gr->vk.swapchain_images);

	watch("%d", gr->vk.swapchain_length);

	for (int n=0; n < gr->vk.swapchain_length; n++) {

		VkImageViewCreateInfo ivci = {};

		ivci.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ivci.image                           = gr->vk.swapchain_images[n];
		ivci.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
		ivci.format                          = gr->vk.swapchain_format;
		ivci.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		ivci.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		ivci.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		ivci.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		ivci.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		ivci.subresourceRange.baseMipLevel   = 0;
		ivci.subresourceRange.levelCount     = 1;
		ivci.subresourceRange.baseArrayLayer = 0;
		ivci.subresourceRange.layerCount     = 1;

		if (vkCreateImageView(gr->vk.gpu, &ivci, NULL, &gr->vk.swapchain_views[n]) != VK_SUCCESS)
			return fail_msg("graphics: vkCreateImageView() failed!");

	}

	log_i("graphics: Swapchain views ready");
	return true;

}



bool init_commandpool(graphics *gr)
{

	VkCommandPoolCreateInfo     cpci = {};
	VkCommandBufferAllocateInfo cbai = {};

	// Create Graphics Pool
	{
		cpci.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cpci.queueFamilyIndex = gr->vk.graphics_queue_index;
		cpci.flags            = 0;

		if (vkCreateCommandPool(gr->vk.gpu, &cpci, NULL, &gr->vk.command_pool) != VK_SUCCESS)
			return fail_msg("graphics: [graphics] vkCreateCommandPool() failed!");

		cbai.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cbai.commandPool        = gr->vk.command_pool;
		cbai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cbai.commandBufferCount = gr->vk.swapchain_length;

		if (vkAllocateCommandBuffers(gr->vk.gpu, &cbai, &gr->vk.command_buffer[0]) != VK_SUCCESS)
			return fail_msg("graphics: [graphics] vkAllocateCommandBuffers() failed!");
	}

	// Create Transfer Pool
	{
		cpci.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cpci.queueFamilyIndex = gr->vk.transfer_queue_index;
		cpci.flags            = 0;

		if (vkCreateCommandPool(gr->vk.gpu, &cpci, NULL, &gr->vk.transfer_pool) != VK_SUCCESS)
			return fail_msg("graphics: [transfer] vkCreateCommandPool() failed!");

		cbai.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cbai.commandPool        = gr->vk.transfer_pool;
		cbai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cbai.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(gr->vk.gpu, &cbai, &gr->vk.transfer_buffer) != VK_SUCCESS)
			return fail_msg("graphics: [transfer] vkAllocateCommandBuffers() failed!");
	}

	log_i("graphics: Command queues ready");

	return true;

}



bool init_synchronization(graphics *gr)
{

	VkSemaphoreCreateInfo sci = {};

	sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(gr->vk.gpu, &sci, NULL, &gr->vk.signal_image_ready) != VK_SUCCESS)
		return fail_msg("graphics: [image] vkCreateSemaphore() failed!");

	if (vkCreateSemaphore(gr->vk.gpu, &sci, NULL, &gr->vk.signal_render_complete) != VK_SUCCESS)
		return fail_msg("graphics: [render] vkCreateSemaphore() failed!");

	return true;

}



bool init_descriptors(graphics *gr)
{

	{

		VkDescriptorPoolSize            dps   = {};
		VkDescriptorPoolCreateInfo      dpci  = {};
		VkDescriptorSetLayoutBinding    dslb  = {};
		VkDescriptorSetLayoutCreateInfo dslci = {};

		dps.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		dps.descriptorCount = 1;

		dpci.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		dpci.poolSizeCount = 1;
		dpci.pPoolSizes    = &dps;
		dpci.maxSets       = 256;

		if (vkCreateDescriptorPool(gr->vk.gpu, &dpci, NULL, &gr->vk.descriptor_uniform_pool) != VK_SUCCESS)
			return fail_msg("graphics: [uniform] vkCreateDescriptorPool() failed!");

		dslb.binding            = 0;
		dslb.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		dslb.descriptorCount    = 1;
		dslb.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		dslb.pImmutableSamplers = NULL;

		dslci.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		dslci.bindingCount = 1;
		dslci.pBindings    = &dslb;

		if (vkCreateDescriptorSetLayout(gr->vk.gpu, &dslci, NULL, &gr->vk.descriptor_uniform_layout) != VK_SUCCESS)
			return fail_msg("graphics: [uniform] vkCreateDescriptorSetLayout() failed!");

	}

	{
		VkDescriptorPoolSize            dps   = {};
		VkDescriptorPoolCreateInfo      dpci  = {};
		VkDescriptorSetLayoutBinding    dslb  = {};
		VkDescriptorSetLayoutCreateInfo dslci = {};

		dps.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		dps.descriptorCount = 1;

		dpci.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		dpci.poolSizeCount = 1;
		dpci.pPoolSizes    = &dps;
		dpci.maxSets       = 256;

		if (vkCreateDescriptorPool(gr->vk.gpu, &dpci, NULL, &gr->vk.descriptor_texture_pool) != VK_SUCCESS)
			return fail_msg("graphics: [texture] vkCreateDescriptorPool() failed!");

		dslb.binding            = 0;
		dslb.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		dslb.descriptorCount    = 1;
		dslb.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
		dslb.pImmutableSamplers = NULL;

		dslci.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		dslci.bindingCount = 1;
		dslci.pBindings    = &dslb;

		if (vkCreateDescriptorSetLayout(gr->vk.gpu, &dslci, NULL, &gr->vk.descriptor_texture_layout) != VK_SUCCESS)
			return fail_msg("graphics: [texture] vkCreateDescriptorSetLayout() failed!");

	}

	const VkDescriptorSetLayout descriptors[] = {
		gr->vk.descriptor_uniform_layout,
		gr->vk.descriptor_texture_layout
	};

	VkPipelineLayoutCreateInfo plci = {};

	plci.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	plci.setLayoutCount         = countof(descriptors);
	plci.pSetLayouts            = &descriptors[0];
	plci.pushConstantRangeCount = 0;
	plci.pPushConstantRanges    = 0;

	VkPipelineLayout layout = NULL;

	if (vkCreatePipelineLayout(gr->vk.gpu, &plci, NULL, &gr->vk.pipeline_layout) != VK_SUCCESS)
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

	log_i("graphics: vulkan(lyr=%s, loc=%d, cod=%d, obj=%d:%ld): %s",
		layer_prefix, location, code, obj_type, obj, msg);

	return VK_FALSE;

}

