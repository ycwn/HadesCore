

#ifndef GR_GRAPHICS_H
#define GR_GRAPHICS_H


#define GR_VKSYM_EXT(sym) extern PFN_##sym sym;
#define GR_VKSYM_DEF(sym) PFN_##sym        sym = NULL;
#define GR_VKSYM(sym)                                                                             \
	do {                                                                                      \
		if ((sym = (PFN_##sym)glfwGetInstanceProcAddress(gfx.vk.instance, #sym)) == NULL) \
			log_d("graphics: " #sym "() == NULL!");                                   \
	} while (0)


GR_VKSYM_EXT(vkAllocateCommandBuffers);
GR_VKSYM_EXT(vkAllocateDescriptorSets);
GR_VKSYM_EXT(vkAllocateMemory);
GR_VKSYM_EXT(vkBeginCommandBuffer);
GR_VKSYM_EXT(vkBindBufferMemory);
GR_VKSYM_EXT(vkBindImageMemory);
GR_VKSYM_EXT(vkCmdBeginRenderPass);
GR_VKSYM_EXT(vkCmdBindDescriptorSets);
GR_VKSYM_EXT(vkCmdBindIndexBuffer);
GR_VKSYM_EXT(vkCmdBindPipeline);
GR_VKSYM_EXT(vkCmdBindVertexBuffers);
GR_VKSYM_EXT(vkCmdCopyBuffer);
GR_VKSYM_EXT(vkCmdCopyBufferToImage);
GR_VKSYM_EXT(vkCmdDraw);
GR_VKSYM_EXT(vkCmdDrawIndexed);
GR_VKSYM_EXT(vkCmdEndRenderPass);
GR_VKSYM_EXT(vkCmdPipelineBarrier);
GR_VKSYM_EXT(vkCreateBuffer);
GR_VKSYM_EXT(vkCreateCommandPool);
GR_VKSYM_EXT(vkCreateDescriptorPool);
GR_VKSYM_EXT(vkCreateDescriptorSetLayout);
GR_VKSYM_EXT(vkCreateDevice);
GR_VKSYM_EXT(vkCreateFence);
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
GR_VKSYM_EXT(vkDestroyDescriptorPool);
GR_VKSYM_EXT(vkDestroyDescriptorSetLayout);
GR_VKSYM_EXT(vkDestroyDevice);
GR_VKSYM_EXT(vkDestroyFence);
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
GR_VKSYM_EXT(vkEndCommandBuffer);
GR_VKSYM_EXT(vkEnumerateDeviceExtensionProperties);
GR_VKSYM_EXT(vkEnumerateInstanceExtensionProperties);
GR_VKSYM_EXT(vkEnumeratePhysicalDevices);
GR_VKSYM_EXT(vkFreeMemory);
GR_VKSYM_EXT(vkGetBufferMemoryRequirements);
GR_VKSYM_EXT(vkGetDeviceQueue);
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
GR_VKSYM_EXT(vkResetFences);
GR_VKSYM_EXT(vkUnmapMemory);
GR_VKSYM_EXT(vkUpdateDescriptorSets);
GR_VKSYM_EXT(vkWaitForFences);

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


typedef struct graphics {

	GLFWwindow *window;

	struct {

		int *screen_width;
		int *screen_height;
		int *screen_fullscreen;

		int *vsync_enable;
		int *vsync_adaptive;

		int *triple_buffer;

		int *validate;

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
		VkFence         command_fence[GR_SWAPCHAIN_MAX];

		VkCommandPool   transfer_pool;
		VkCommandBuffer transfer_buffer;

		VkSemaphore signal_image_ready;
		VkSemaphore signal_render_complete;

		VkDescriptorPool descriptor_uniform_pool;
		VkDescriptorPool descriptor_texture_pool;

		VkDescriptorSetLayout descriptor_uniform_layout;
		VkDescriptorSetLayout descriptor_texture_layout;

		VkPipelineLayout pipeline_layout;

	} vk;

} graphics;


graphics *gr_create();
void      gr_destroy();

bool gr_request_instance_extension(const char *ext);
bool gr_request_device_extension(  const char *ext);

bool gr_set_video();
bool gr_build_pipeline(const char *file);
void gr_submit();

int  gr_get_memory_type(uint mask, uint props);
bool gr_create_buffer(VkBuffer *buf, VkDeviceMemory *mem, size_t size, uint usage, uint props);
bool gr_create_image( VkImage  *img, VkDeviceMemory *mem, VkFormat format, uint width, uint height, uint depth,	uint mipmaps, uint layers, uint tiling, uint usage, uint props, uint flags);
bool gr_create_image_view(VkImageView *view, VkImage img, VkFormat fmt, VkImageAspectFlags asp);
void gr_transfer_begin();
void gr_transfer_end();
void gr_upload(const void *src, VkDeviceMemory dst, size_t len);
void gr_copy_buffer_to_buffer(VkBuffer src, VkBuffer dst, size_t len);
void gr_copy_buffer_to_image( VkBuffer src,  VkImage  dst, uint width, uint height, uint depth);
void gr_mcopy_buffer_to_buffer(VkBuffer src, VkBuffer dst, uint skip[],  uint offset[], size_t length[], uint count);
void gr_mcopy_buffer_to_image( VkBuffer src, VkImage  dst, uint width, uint height, uint depth, uint offset[], uint mipmap[], uint layer[], uint count);
void gr_transition_layout(VkImage img, VkImageLayout layout, VkImageAspectFlags mask, VkAccessFlagBits src, VkAccessFlagBits dst);


#endif

