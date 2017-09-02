

#ifndef __GR_GRAPHICS_H
#define __GR_GRAPHICS_H


#define GR_VKSYM_EXT(sym) extern PFN_##sym sym;
#define GR_VKSYM_DEF(sym) PFN_##sym sym = NULL;
#define GR_VKSYM(sym)     if ((sym = (PFN_##sym)vkGetInstanceProcAddr(gr->vk.instance, #sym)) == NULL) log_d("graphics: " #sym "() == NULL!");

#define GR_SWAPCHAIN_MAX      4
#define GR_INSTANCE_EXT_MAX  64
#define GR_DEVICE_EXT_MAX    64

GR_VKSYM_EXT(vkAllocateCommandBuffers);
GR_VKSYM_EXT(vkAllocateDescriptorSets);
GR_VKSYM_EXT(vkAllocateMemory);
GR_VKSYM_EXT(vkBeginCommandBuffer);
GR_VKSYM_EXT(vkBindBufferMemory);
GR_VKSYM_EXT(vkBindImageMemory);
GR_VKSYM_EXT(vkCmdBeginRenderPass);
GR_VKSYM_EXT(vkCmdBindDescriptorSets);
GR_VKSYM_EXT(vkCmdBindPipeline);
GR_VKSYM_EXT(vkCmdBindIndexBuffer);
GR_VKSYM_EXT(vkCmdBindVertexBuffers);
GR_VKSYM_EXT(vkCmdCopyBuffer);
GR_VKSYM_EXT(vkCmdCopyBufferToImage);
GR_VKSYM_EXT(vkCmdDraw);
GR_VKSYM_EXT(vkCmdDrawIndexed);
GR_VKSYM_EXT(vkCmdEndRenderPass);
GR_VKSYM_EXT(vkCmdPipelineBarrier);
GR_VKSYM_EXT(vkCreateBuffer);
GR_VKSYM_EXT(vkCreateCommandPool);
GR_VKSYM_EXT(vkCreateDescriptorSetLayout);
GR_VKSYM_EXT(vkCreateDescriptorPool);
GR_VKSYM_EXT(vkCreateDevice);
GR_VKSYM_EXT(vkCreateFramebuffer);
GR_VKSYM_EXT(vkCreateGraphicsPipelines);
GR_VKSYM_EXT(vkCreateImage);
GR_VKSYM_EXT(vkCreateImageView);
GR_VKSYM_EXT(vkCreateInstance);
GR_VKSYM_EXT(vkCreatePipelineLayout);
GR_VKSYM_EXT(vkCreateRenderPass);
GR_VKSYM_EXT(vkCreateSampler);
GR_VKSYM_EXT(vkCreateSemaphore);
GR_VKSYM_EXT(vkCreateShaderModule);
GR_VKSYM_EXT(vkDestroyBuffer);
GR_VKSYM_EXT(vkDestroyCommandPool);
GR_VKSYM_EXT(vkDestroyDescriptorSetLayout);
GR_VKSYM_EXT(vkDestroyDescriptorPool);
GR_VKSYM_EXT(vkDestroyDevice);
GR_VKSYM_EXT(vkDestroyFramebuffer);
GR_VKSYM_EXT(vkDestroyImage);
GR_VKSYM_EXT(vkDestroyImageView);
GR_VKSYM_EXT(vkDestroyInstance);
GR_VKSYM_EXT(vkDestroyPipeline);
GR_VKSYM_EXT(vkDestroyPipelineLayout);
GR_VKSYM_EXT(vkDestroyRenderPass);
GR_VKSYM_EXT(vkDestroySampler);
GR_VKSYM_EXT(vkDestroySemaphore);
GR_VKSYM_EXT(vkDestroyShaderModule);
GR_VKSYM_EXT(vkFreeMemory);
GR_VKSYM_EXT(vkEndCommandBuffer);
GR_VKSYM_EXT(vkEnumerateDeviceExtensionProperties);
GR_VKSYM_EXT(vkEnumerateInstanceExtensionProperties);
GR_VKSYM_EXT(vkEnumeratePhysicalDevices);
GR_VKSYM_EXT(vkGetDeviceQueue);
GR_VKSYM_EXT(vkGetBufferMemoryRequirements);
GR_VKSYM_EXT(vkGetImageMemoryRequirements);
GR_VKSYM_EXT(vkGetInstanceProcAddr);
GR_VKSYM_EXT(vkGetPhysicalDeviceFeatures);
GR_VKSYM_EXT(vkGetPhysicalDeviceFormatProperties);
GR_VKSYM_EXT(vkGetPhysicalDeviceMemoryProperties);
GR_VKSYM_EXT(vkGetPhysicalDeviceProperties);
GR_VKSYM_EXT(vkGetPhysicalDeviceQueueFamilyProperties);
GR_VKSYM_EXT(vkMapMemory);
GR_VKSYM_EXT(vkQueueSubmit);
GR_VKSYM_EXT(vkQueueWaitIdle);
GR_VKSYM_EXT(vkUnmapMemory);
GR_VKSYM_EXT(vkUpdateDescriptorSets);

GR_VKSYM_EXT(vkAcquireNextImageKHR);
GR_VKSYM_EXT(vkCreateSwapchainKHR);
GR_VKSYM_EXT(vkDestroySurfaceKHR);
GR_VKSYM_EXT(vkDestroySwapchainKHR);
GR_VKSYM_EXT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
GR_VKSYM_EXT(vkGetPhysicalDeviceSurfaceFormatsKHR);
GR_VKSYM_EXT(vkGetPhysicalDeviceSurfacePresentModesKHR);
GR_VKSYM_EXT(vkGetPhysicalDeviceSurfaceSupportKHR);
GR_VKSYM_EXT(vkGetSwapchainImagesKHR);
GR_VKSYM_EXT(vkQueuePresentKHR);
GR_VKSYM_EXT(vkCreateDebugReportCallbackEXT);
GR_VKSYM_EXT(vkDestroyDebugReportCallbackEXT);


typedef struct _graphics_t {

	SDL_Window *window;

	struct {

		variable *screen_width;
		variable *screen_height;
		variable *screen_fullscreen;

		variable *vsync_enable;
		variable *vsync_adaptive;

		variable *triple_buffer;

		variable *validate;

	} var;

	struct {

		const char *instance[GR_DEVICE_EXT_MAX];
		uint        instance_num;

		const char *device[GR_DEVICE_EXT_MAX];
		uint        device_num;

	} ext;

	struct {

		VkInstance instance;
		VkDevice   gpu;

		VkDebugReportCallbackEXT callback;

		VkPhysicalDevice                 device;
		VkPhysicalDeviceProperties       device_properties;
		VkPhysicalDeviceFeatures         device_features;
		VkPhysicalDeviceMemoryProperties memory_properties;

		int     graphics_queue_index; //FIXME: Fold all queues into one, to prevent sync errors
		int     graphics_queue_count;
		VkQueue graphics_queue;

		int     compute_queue_index;
		int     compute_queue_count;
		VkQueue compute_queue;

		int     transfer_queue_index;
		int     transfer_queue_count;
		VkQueue transfer_queue;

		int     presentation_queue_index;
		int     presentation_queue_count;
		VkQueue presentation_queue;

		VkSurfaceKHR surface;

		int            swapchain_width;
		int            swapchain_height;
		int            swapchain_length;
		int            swapchain_curr;
		VkImage        swapchain_images[GR_SWAPCHAIN_MAX];
		VkImageView    swapchain_views[GR_SWAPCHAIN_MAX];
		VkFormat       swapchain_format;
		VkSwapchainKHR swapchain;

		VkCommandPool   command_pool;
		VkCommandBuffer command_buffer[GR_SWAPCHAIN_MAX];

		VkCommandPool   transfer_pool;
		VkCommandBuffer transfer_buffer;

		VkSemaphore signal_image_ready;
		VkSemaphore signal_render_complete;

		VkDescriptorPool descriptor_uniform_pool;
		VkDescriptorPool descriptor_texture_pool;

		VkDescriptorSetLayout descriptor_uniform_layout;
		VkDescriptorSetLayout descriptor_texture_layout;

	} vk;

} graphics;


graphics *gr_create();
void      gr_destroy(graphics *gr);

bool gr_request_instance_extension(graphics *gr, const char *ext);
bool gr_request_device_extension(  graphics *gr, const char *ext);

bool gr_set_video(graphics *gr);
void gr_submit(graphics *gr);


#endif


