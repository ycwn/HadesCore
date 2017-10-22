

import hades.core_variable;

import hades.graphics_limits;
import hades.graphics_types;


struct graphics {

	private struct vars {

		variable *screen_width;
		variable *screen_height;
		variable *screen_fullscreen;

		variable *vsync_enable;
		variable *vsync_adaptive;

		variable *triple_buffer;

		variable *validate;

	}

	private struct exts {

		const(char)*[GR_DEVICE_EXT_MAX] instance;
		uint                            instance_num;

		const(char)*[GR_DEVICE_EXT_MAX] device;
		uint                            device_num;

	}

	private struct vks {

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

		int                            swapchain_width;
		int                            swapchain_height;
		int                            swapchain_length;
		int                            swapchain_curr;
		VkImage[GR_SWAPCHAIN_MAX]      swapchain_images;
		VkImageView[GR_SWAPCHAIN_MAX]  swapchain_views;
		VkFormat                       swapchain_format;
		VkSwapchainKHR                 swapchain;

		VkCommandPool                     command_pool;
		VkCommandBuffer[GR_SWAPCHAIN_MAX] command_buffer;
		VkFence[GR_SWAPCHAIN_MAX]         command_fence;

		VkCommandPool   transfer_pool;
		VkCommandBuffer transfer_buffer;

		VkSemaphore signal_image_ready;
		VkSemaphore signal_render_complete;

		VkDescriptorPool descriptor_uniform_pool;
		VkDescriptorPool descriptor_texture_pool;

		VkDescriptorSetLayout descriptor_uniform_layout;
		VkDescriptorSetLayout descriptor_texture_layout;

		VkPipelineLayout pipeline_layout;

	}


	GLFWwindow *window;

	vars var;
	exts ext;
	vks  vk;

}


extern(C) {

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
	bool gr_create_image_view(VkImageView *view, VkImage img, VkFormat fmt);
	void gr_transfer_begin();
	void gr_transfer_end();
	void gr_upload(const void *src, VkDeviceMemory dst, size_t len);
	void gr_copy_buffer_to_buffer(VkBuffer src, VkBuffer dst, size_t len);
	void gr_copy_buffer_to_image( VkBuffer src,  VkImage  dst, uint width, uint height, uint depth);
	void gr_mcopy_buffer_to_buffer(VkBuffer src, VkBuffer dst, uint *skip,  uint *offset, size_t *length, uint count);
	void gr_mcopy_buffer_to_image( VkBuffer src, VkImage  dst, uint width, uint height, uint depth, uint *offset, uint *mipmap, uint *layer, uint count);
	void gr_transition_layout(VkImage img, VkImageLayout layout, VkAccessFlagBits src, VkAccessFlagBits dst);

}

