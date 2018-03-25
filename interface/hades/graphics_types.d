

enum VK_MAX_PHYSICAL_DEVICE_NAME_SIZE = 256;
enum VK_UUID_SIZE                     = 16;
enum VK_MAX_MEMORY_TYPES              = 32;
enum VK_MAX_MEMORY_HEAPS              = 16;
enum VK_MAX_EXTENSION_NAME_SIZE       = 256;
enum VK_MAX_DESCRIPTION_SIZE          = 256;


alias VkFlags      = uint;
alias VkBool32     = uint;
alias VkDeviceSize = ulong;
alias VkSampleMask = uint;

alias GLFWwindow = void;

alias VkBuffer              = void*;
alias VkBufferView          = void*;
alias VkCommandBuffer       = void*;
alias VkCommandPool         = void*;
alias VkDescriptorPool      = void*;
alias VkDescriptorSet       = void*;
alias VkDescriptorSetLayout = void*;
alias VkDevice              = void*;
alias VkDeviceMemory        = void*;
alias VkEvent               = void*;
alias VkFence               = void*;
alias VkFramebuffer         = void*;
alias VkImage               = void*;
alias VkImageView           = void*;
alias VkInstance            = void*;
alias VkPhysicalDevice      = void*;
alias VkPipeline            = void*;
alias VkPipelineCache       = void*;
alias VkPipelineLayout      = void*;
alias VkQueryPool           = void*;
alias VkQueue               = void*;
alias VkRenderPass          = void*;
alias VkSampler             = void*;
alias VkSemaphore           = void*;
alias VkShaderModule        = void*;

alias VkSurfaceKHR   = void*;
alias VkSwapchainKHR = void*;

alias VkDebugReportCallbackEXT = void*;

alias VkAccessFlagBits                         = uint;
alias VkAttachmentDescriptionFlagBits          = uint;
alias VkAttachmentLoadOp                       = uint;
alias VkAttachmentStoreOp                      = uint;
alias VkBlendFactor                            = uint;
alias VkBlendOp                                = uint;
alias VkBlendOverlapEXT                        = uint;
alias VkBorderColor                            = uint;
alias VkBufferCreateFlagBits                   = uint;
alias VkBufferUsageFlagBits                    = uint;
alias VkChromaLocationKHR                      = uint;
alias VkColorComponentFlagBits                 = uint;
alias VkColorSpaceKHR                          = uint;
alias VkCommandBufferLevel                     = uint;
alias VkCommandBufferResetFlagBits             = uint;
alias VkCommandBufferUsageFlagBits             = uint;
alias VkCommandPoolCreateFlagBits              = uint;
alias VkCommandPoolResetFlagBits               = uint;
alias VkCompareOp                              = uint;
alias VkComponentSwizzle                       = uint;
alias VkCompositeAlphaFlagBitsKHR              = uint;
alias VkCoverageModulationModeNV               = uint;
alias VkCullModeFlagBits                       = uint;
alias VkDebugReportFlagBitsEXT                 = uint;
alias VkDebugReportObjectTypeEXT               = uint;
alias VkDependencyFlagBits                     = uint;
alias VkDescriptorPoolCreateFlagBits           = uint;
alias VkDescriptorSetLayoutCreateFlagBits      = uint;
alias VkDescriptorType                         = uint;
alias VkDescriptorUpdateTemplateTypeKHR        = uint;
alias VkDeviceEventTypeEXT                     = uint;
alias VkDeviceGroupPresentModeFlagBitsKHX      = uint;
alias VkDiscardRectangleModeEXT                = uint;
alias VkDisplayEventTypeEXT                    = uint;
alias VkDisplayPlaneAlphaFlagBitsKHR           = uint;
alias VkDisplayPowerStateEXT                   = uint;
alias VkDynamicState                           = uint;
alias VkExternalFenceFeatureFlagBitsKHR        = uint;
alias VkExternalFenceHandleTypeFlagBitsKHR     = uint;
alias VkExternalMemoryFeatureFlagBitsKHR       = uint;
alias VkExternalMemoryFeatureFlagBitsNV        = uint;
alias VkExternalMemoryHandleTypeFlagBitsKHR    = uint;
alias VkExternalMemoryHandleTypeFlagBitsNV     = uint;
alias VkExternalSemaphoreFeatureFlagBitsKHR    = uint;
alias VkExternalSemaphoreHandleTypeFlagBitsKHR = uint;
alias VkFenceCreateFlagBits                    = uint;
alias VkFenceImportFlagBitsKHR                 = uint;
alias VkFilter                                 = uint;
alias VkFormat                                 = uint;
alias VkFormatFeatureFlagBits                  = uint;
alias VkFrontFace                              = uint;
alias VkImageAspectFlagBits                    = uint;
alias VkImageCreateFlagBits                    = uint;
alias VkImageLayout                            = uint;
alias VkImageTiling                            = uint;
alias VkImageType                              = uint;
alias VkImageUsageFlagBits                     = uint;
alias VkImageViewType                          = uint;
alias VkIndexType                              = uint;
alias VkIndirectCommandsLayoutUsageFlagBitsNVX = uint;
alias VkIndirectCommandsTokenTypeNVX           = uint;
alias VkInternalAllocationType                 = uint;
alias VkLogicOp                                = uint;
alias VkMemoryAllocateFlagBitsKHX              = uint;
alias VkMemoryHeapFlagBits                     = uint;
alias VkMemoryPropertyFlagBits                 = uint;
alias VkObjectEntryTypeNVX                     = uint;
alias VkObjectEntryUsageFlagBitsNVX            = uint;
alias VkObjectType                             = uint;
alias VkPeerMemoryFeatureFlagBitsKHX           = uint;
alias VkPhysicalDeviceType                     = uint;
alias VkPipelineBindPoint                      = uint;
alias VkPipelineCacheHeaderVersion             = uint;
alias VkPipelineCreateFlagBits                 = uint;
alias VkPipelineStageFlagBits                  = uint;
alias VkPointClippingBehaviorKHR               = uint;
alias VkPolygonMode                            = uint;
alias VkPresentModeKHR                         = uint;
alias VkPrimitiveTopology                      = uint;
alias VkQueryControlFlagBits                   = uint;
alias VkQueryPipelineStatisticFlagBits         = uint;
alias VkQueryResultFlagBits                    = uint;
alias VkQueryType                              = uint;
alias VkQueueFlagBits                          = uint;
alias VkRasterizationOrderAMD                  = uint;
alias VkResult                                 = uint;
alias VkSampleCountFlagBits                    = uint;
alias VkSamplerAddressMode                     = uint;
alias VkSamplerMipmapMode                      = uint;
alias VkSamplerReductionModeEXT                = uint;
alias VkSamplerYcbcrModelConversionKHR         = uint;
alias VkSamplerYcbcrRangeKHR                   = uint;
alias VkSemaphoreImportFlagBitsKHR             = uint;
alias VkShaderStageFlagBits                    = uint;
alias VkSharingMode                            = uint;
alias VkSparseImageFormatFlagBits              = uint;
alias VkSparseMemoryBindFlagBits               = uint;
alias VkStencilFaceFlagBits                    = uint;
alias VkStencilOp                              = uint;
alias VkStructureType                          = uint;
alias VkSubpassContents                        = uint;
alias VkSubpassDescriptionFlagBits             = uint;
alias VkSurfaceCounterFlagBitsEXT              = uint;
alias VkSurfaceTransformFlagBitsKHR            = uint;
alias VkSwapchainCreateFlagBitsKHR             = uint;
alias VkSystemAllocationScope                  = uint;
alias VkTessellationDomainOriginKHR            = uint;
alias VkValidationCacheHeaderVersionEXT        = uint;
alias VkValidationCheckEXT                     = uint;
alias VkVertexInputRate                        = uint;
alias VkViewportCoordinateSwizzleNV            = uint;

alias VkAccessFlags                                  = VkFlags;
alias VkAndroidSurfaceCreateFlagsKHR                 = VkFlags;
alias VkAttachmentDescriptionFlags                   = VkFlags;
alias VkBufferCreateFlags                            = VkFlags;
alias VkBufferUsageFlags                             = VkFlags;
alias VkBufferViewCreateFlags                        = VkFlags;
alias VkColorComponentFlags                          = VkFlags;
alias VkCommandBufferResetFlags                      = VkFlags;
alias VkCommandBufferUsageFlags                      = VkFlags;
alias VkCommandPoolCreateFlags                       = VkFlags;
alias VkCommandPoolResetFlags                        = VkFlags;
alias VkCommandPoolTrimFlagsKHR                      = VkFlags;
alias VkCompositeAlphaFlagsKHR                       = VkFlags;
alias VkCullModeFlags                                = VkFlags;
alias VkDebugReportFlagsEXT                          = VkFlags;
alias VkDependencyFlags                              = VkFlags;
alias VkDescriptorPoolCreateFlags                    = VkFlags;
alias VkDescriptorPoolResetFlags                     = VkFlags;
alias VkDescriptorSetLayoutCreateFlags               = VkFlags;
alias VkDescriptorUpdateTemplateCreateFlagsKHR       = VkFlags;
alias VkDeviceCreateFlags                            = VkFlags;
alias VkDeviceGroupPresentModeFlagsKHX               = VkFlags;
alias VkDeviceQueueCreateFlags                       = VkFlags;
alias VkDisplayModeCreateFlagsKHR                    = VkFlags;
alias VkDisplayPlaneAlphaFlagsKHR                    = VkFlags;
alias VkDisplaySurfaceCreateFlagsKHR                 = VkFlags;
alias VkEventCreateFlags                             = VkFlags;
alias VkExternalFenceFeatureFlagsKHR                 = VkFlags;
alias VkExternalFenceHandleTypeFlagsKHR              = VkFlags;
alias VkExternalMemoryFeatureFlagsKHR                = VkFlags;
alias VkExternalMemoryFeatureFlagsNV                 = VkFlags;
alias VkExternalMemoryHandleTypeFlagsKHR             = VkFlags;
alias VkExternalMemoryHandleTypeFlagsNV              = VkFlags;
alias VkExternalSemaphoreFeatureFlagsKHR             = VkFlags;
alias VkExternalSemaphoreHandleTypeFlagsKHR          = VkFlags;
alias VkFenceCreateFlags                             = VkFlags;
alias VkFenceImportFlagsKHR                          = VkFlags;
alias VkFormatFeatureFlags                           = VkFlags;
alias VkFramebufferCreateFlags                       = VkFlags;
alias VkIOSSurfaceCreateFlagsMVK                     = VkFlags;
alias VkImageAspectFlags                             = VkFlags;
alias VkImageCreateFlags                             = VkFlags;
alias VkImageUsageFlags                              = VkFlags;
alias VkImageViewCreateFlags                         = VkFlags;
alias VkIndirectCommandsLayoutUsageFlagsNVX          = VkFlags;
alias VkInstanceCreateFlags                          = VkFlags;
alias VkMacOSSurfaceCreateFlagsMVK                   = VkFlags;
alias VkMemoryAllocateFlagsKHX                       = VkFlags;
alias VkMemoryHeapFlags                              = VkFlags;
alias VkMemoryMapFlags                               = VkFlags;
alias VkMemoryPropertyFlags                          = VkFlags;
alias VkMirSurfaceCreateFlagsKHR                     = VkFlags;
alias VkObjectEntryUsageFlagsNVX                     = VkFlags;
alias VkPeerMemoryFeatureFlagsKHX                    = VkFlags;
alias VkPipelineCacheCreateFlags                     = VkFlags;
alias VkPipelineColorBlendStateCreateFlags           = VkFlags;
alias VkPipelineCoverageModulationStateCreateFlagsNV = VkFlags;
alias VkPipelineCoverageToColorStateCreateFlagsNV    = VkFlags;
alias VkPipelineCreateFlags                          = VkFlags;
alias VkPipelineDepthStencilStateCreateFlags         = VkFlags;
alias VkPipelineDiscardRectangleStateCreateFlagsEXT  = VkFlags;
alias VkPipelineDynamicStateCreateFlags              = VkFlags;
alias VkPipelineInputAssemblyStateCreateFlags        = VkFlags;
alias VkPipelineLayoutCreateFlags                    = VkFlags;
alias VkPipelineMultisampleStateCreateFlags          = VkFlags;
alias VkPipelineRasterizationStateCreateFlags        = VkFlags;
alias VkPipelineShaderStageCreateFlags               = VkFlags;
alias VkPipelineStageFlags                           = VkFlags;
alias VkPipelineTessellationStateCreateFlags         = VkFlags;
alias VkPipelineVertexInputStateCreateFlags          = VkFlags;
alias VkPipelineViewportStateCreateFlags             = VkFlags;
alias VkPipelineViewportSwizzleStateCreateFlagsNV    = VkFlags;
alias VkQueryControlFlags                            = VkFlags;
alias VkQueryPipelineStatisticFlags                  = VkFlags;
alias VkQueryPoolCreateFlags                         = VkFlags;
alias VkQueryResultFlags                             = VkFlags;
alias VkQueueFlags                                   = VkFlags;
alias VkRenderPassCreateFlags                        = VkFlags;
alias VkSampleCountFlags                             = VkFlags;
alias VkSamplerCreateFlags                           = VkFlags;
alias VkSemaphoreCreateFlags                         = VkFlags;
alias VkSemaphoreImportFlagsKHR                      = VkFlags;
alias VkShaderModuleCreateFlags                      = VkFlags;
alias VkShaderStageFlags                             = VkFlags;
alias VkSparseImageFormatFlags                       = VkFlags;
alias VkSparseMemoryBindFlags                        = VkFlags;
alias VkStencilFaceFlags                             = VkFlags;
alias VkSubpassDescriptionFlags                      = VkFlags;
alias VkSurfaceCounterFlagsEXT                       = VkFlags;
alias VkSurfaceTransformFlagsKHR                     = VkFlags;
alias VkSwapchainCreateFlagsKHR                      = VkFlags;
alias VkValidationCacheCreateFlagsEXT                = VkFlags;
alias VkViSurfaceCreateFlagsNN                       = VkFlags;
alias VkWaylandSurfaceCreateFlagsKHR                 = VkFlags;
alias VkWin32SurfaceCreateFlagsKHR                   = VkFlags;
alias VkXcbSurfaceCreateFlagsKHR                     = VkFlags;
alias VkXlibSurfaceCreateFlagsKHR                    = VkFlags;


struct VkFormatProperties {
    VkFormatFeatureFlags  linearTilingFeatures;
    VkFormatFeatureFlags  optimalTilingFeatures;
    VkFormatFeatureFlags  bufferFeatures;
}

struct VkPhysicalDeviceLimits {
    uint                  maxImageDimension1D;
    uint                  maxImageDimension2D;
    uint                  maxImageDimension3D;
    uint                  maxImageDimensionCube;
    uint                  maxImageArrayLayers;
    uint                  maxTexelBufferElements;
    uint                  maxUniformBufferRange;
    uint                  maxStorageBufferRange;
    uint                  maxPushConstantsSize;
    uint                  maxMemoryAllocationCount;
    uint                  maxSamplerAllocationCount;
    VkDeviceSize          bufferImageGranularity;
    VkDeviceSize          sparseAddressSpaceSize;
    uint                  maxBoundDescriptorSets;
    uint                  maxPerStageDescriptorSamplers;
    uint                  maxPerStageDescriptorUniformBuffers;
    uint                  maxPerStageDescriptorStorageBuffers;
    uint                  maxPerStageDescriptorSampledImages;
    uint                  maxPerStageDescriptorStorageImages;
    uint                  maxPerStageDescriptorInputAttachments;
    uint                  maxPerStageResources;
    uint                  maxDescriptorSetSamplers;
    uint                  maxDescriptorSetUniformBuffers;
    uint                  maxDescriptorSetUniformBuffersDynamic;
    uint                  maxDescriptorSetStorageBuffers;
    uint                  maxDescriptorSetStorageBuffersDynamic;
    uint                  maxDescriptorSetSampledImages;
    uint                  maxDescriptorSetStorageImages;
    uint                  maxDescriptorSetInputAttachments;
    uint                  maxVertexInputAttributes;
    uint                  maxVertexInputBindings;
    uint                  maxVertexInputAttributeOffset;
    uint                  maxVertexInputBindingStride;
    uint                  maxVertexOutputComponents;
    uint                  maxTessellationGenerationLevel;
    uint                  maxTessellationPatchSize;
    uint                  maxTessellationControlPerVertexInputComponents;
    uint                  maxTessellationControlPerVertexOutputComponents;
    uint                  maxTessellationControlPerPatchOutputComponents;
    uint                  maxTessellationControlTotalOutputComponents;
    uint                  maxTessellationEvaluationInputComponents;
    uint                  maxTessellationEvaluationOutputComponents;
    uint                  maxGeometryShaderInvocations;
    uint                  maxGeometryInputComponents;
    uint                  maxGeometryOutputComponents;
    uint                  maxGeometryOutputVertices;
    uint                  maxGeometryTotalOutputComponents;
    uint                  maxFragmentInputComponents;
    uint                  maxFragmentOutputAttachments;
    uint                  maxFragmentDualSrcAttachments;
    uint                  maxFragmentCombinedOutputResources;
    uint                  maxComputeSharedMemorySize;
    uint[3]               maxComputeWorkGroupCount;
    uint                  maxComputeWorkGroupInvocations;
    uint[3]               maxComputeWorkGroupSize;
    uint                  subPixelPrecisionBits;
    uint                  subTexelPrecisionBits;
    uint                  mipmapPrecisionBits;
    uint                  maxDrawIndexedIndexValue;
    uint                  maxDrawIndirectCount;
    float                 maxSamplerLodBias;
    float                 maxSamplerAnisotropy;
    uint                  maxViewports;
    uint[2]               maxViewportDimensions;
    float[2]              viewportBoundsRange;
    uint                  viewportSubPixelBits;
    size_t                minMemoryMapAlignment;
    VkDeviceSize          minTexelBufferOffsetAlignment;
    VkDeviceSize          minUniformBufferOffsetAlignment;
    VkDeviceSize          minStorageBufferOffsetAlignment;
    int                   minTexelOffset;
    uint                  maxTexelOffset;
    int                   minTexelGatherOffset;
    uint                  maxTexelGatherOffset;
    float                 minInterpolationOffset;
    float                 maxInterpolationOffset;
    uint                  subPixelInterpolationOffsetBits;
    uint                  maxFramebufferWidth;
    uint                  maxFramebufferHeight;
    uint                  maxFramebufferLayers;
    VkSampleCountFlags    framebufferColorSampleCounts;
    VkSampleCountFlags    framebufferDepthSampleCounts;
    VkSampleCountFlags    framebufferStencilSampleCounts;
    VkSampleCountFlags    framebufferNoAttachmentsSampleCounts;
    uint                  maxColorAttachments;
    VkSampleCountFlags    sampledImageColorSampleCounts;
    VkSampleCountFlags    sampledImageIntegerSampleCounts;
    VkSampleCountFlags    sampledImageDepthSampleCounts;
    VkSampleCountFlags    sampledImageStencilSampleCounts;
    VkSampleCountFlags    storageImageSampleCounts;
    uint                  maxSampleMaskWords;
    VkBool32              timestampComputeAndGraphics;
    float                 timestampPeriod;
    uint                  maxClipDistances;
    uint                  maxCullDistances;
    uint                  maxCombinedClipAndCullDistances;
    uint                  discreteQueuePriorities;
    float[2]               pointSizeRange;
    float[2]              lineWidthRange;
    float                 pointSizeGranularity;
    float                 lineWidthGranularity;
    VkBool32              strictLines;
    VkBool32              standardSampleLocations;
    VkDeviceSize          optimalBufferCopyOffsetAlignment;
    VkDeviceSize          optimalBufferCopyRowPitchAlignment;
    VkDeviceSize          nonCoherentAtomSize;
}

struct VkPhysicalDeviceSparseProperties {
    VkBool32  residencyStandard2DBlockShape;
    VkBool32  residencyStandard2DMultisampleBlockShape;
    VkBool32  residencyStandard3DBlockShape;
    VkBool32  residencyAlignedMipSize;
    VkBool32  residencyNonResidentStrict;
}

struct VkPhysicalDeviceProperties {
    uint                                    apiVersion;
    uint                                    driverVersion;
    uint                                    vendorID;
    uint                                    deviceID;
    VkPhysicalDeviceType                    deviceType;
    char[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE]  deviceName;
    ubyte[VK_UUID_SIZE]                     pipelineCacheUUID;
    VkPhysicalDeviceLimits                  limits;
    VkPhysicalDeviceSparseProperties        sparseProperties;
}

struct VkPhysicalDeviceFeatures {
    VkBool32    robustBufferAccess;
    VkBool32    fullDrawIndexUint32;
    VkBool32    imageCubeArray;
    VkBool32    independentBlend;
    VkBool32    geometryShader;
    VkBool32    tessellationShader;
    VkBool32    sampleRateShading;
    VkBool32    dualSrcBlend;
    VkBool32    logicOp;
    VkBool32    multiDrawIndirect;
    VkBool32    drawIndirectFirstInstance;
    VkBool32    depthClamp;
    VkBool32    depthBiasClamp;
    VkBool32    fillModeNonSolid;
    VkBool32    depthBounds;
    VkBool32    wideLines;
    VkBool32    largePoints;
    VkBool32    alphaToOne;
    VkBool32    multiViewport;
    VkBool32    samplerAnisotropy;
    VkBool32    textureCompressionETC2;
    VkBool32    textureCompressionASTC_LDR;
    VkBool32    textureCompressionBC;
    VkBool32    occlusionQueryPrecise;
    VkBool32    pipelineStatisticsQuery;
    VkBool32    vertexPipelineStoresAndAtomics;
    VkBool32    fragmentStoresAndAtomics;
    VkBool32    shaderTessellationAndGeometryPointSize;
    VkBool32    shaderImageGatherExtended;
    VkBool32    shaderStorageImageExtendedFormats;
    VkBool32    shaderStorageImageMultisample;
    VkBool32    shaderStorageImageReadWithoutFormat;
    VkBool32    shaderStorageImageWriteWithoutFormat;
    VkBool32    shaderUniformBufferArrayDynamicIndexing;
    VkBool32    shaderSampledImageArrayDynamicIndexing;
    VkBool32    shaderStorageBufferArrayDynamicIndexing;
    VkBool32    shaderStorageImageArrayDynamicIndexing;
    VkBool32    shaderClipDistance;
    VkBool32    shaderCullDistance;
    VkBool32    shaderFloat64;
    VkBool32    shaderInt64;
    VkBool32    shaderInt16;
    VkBool32    shaderResourceResidency;
    VkBool32    shaderResourceMinLod;
    VkBool32    sparseBinding;
    VkBool32    sparseResidencyBuffer;
    VkBool32    sparseResidencyImage2D;
    VkBool32    sparseResidencyImage3D;
    VkBool32    sparseResidency2Samples;
    VkBool32    sparseResidency4Samples;
    VkBool32    sparseResidency8Samples;
    VkBool32    sparseResidency16Samples;
    VkBool32    sparseResidencyAliased;
    VkBool32    variableMultisampleRate;
    VkBool32    inheritedQueries;
}

struct VkMemoryType {
    VkMemoryPropertyFlags  propertyFlags;
    uint                   heapIndex;
}

struct VkMemoryHeap {
    VkDeviceSize       size;
    VkMemoryHeapFlags  flags;
}

struct VkPhysicalDeviceMemoryProperties {
    uint                               memoryTypeCount;
    VkMemoryType[VK_MAX_MEMORY_TYPES]  memoryTypes;
    uint                               memoryHeapCount;
    VkMemoryHeap[VK_MAX_MEMORY_HEAPS]  memoryHeaps;
}

struct VkSpecializationMapEntry {
    uint    constantID;
    uint    offset;
    size_t  size;
}

struct VkSpecializationInfo {
    uint                             mapEntryCount;
    const(VkSpecializationMapEntry) *pMapEntries;
    size_t                           dataSize;
    const(void)                     *pData;
}

struct VkPipelineShaderStageCreateInfo {
    VkStructureType                   sType;
    const(void)                      *pNext;
    VkPipelineShaderStageCreateFlags  flags;
    VkShaderStageFlagBits             stage;
    VkShaderModule                    shmod;
    const(char)                      *pName;
    const(VkSpecializationInfo)      *pSpecializationInfo;
}

struct VkVertexInputBindingDescription {
    uint               binding;
    uint               stride;
    VkVertexInputRate  inputRate;
}

struct VkVertexInputAttributeDescription {
    uint      location;
    uint      binding;
    VkFormat  format;
    uint      offset;
}

struct VkPipelineVertexInputStateCreateInfo {
    VkStructureType                           sType;
    const(void)                              *pNext;
    VkPipelineVertexInputStateCreateFlags     flags;
    uint                                      vertexBindingDescriptionCount;
    const(VkVertexInputBindingDescription)   *pVertexBindingDescriptions;
    uint                                      vertexAttributeDescriptionCount;
    const(VkVertexInputAttributeDescription) *pVertexAttributeDescriptions;
}

struct VkPipelineInputAssemblyStateCreateInfo {
    VkStructureType                          sType;
    const(void)                             *pNext;
    VkPipelineInputAssemblyStateCreateFlags  flags;
    VkPrimitiveTopology                      topology;
    VkBool32                                 primitiveRestartEnable;
}

struct VkPipelineTessellationStateCreateInfo {
    VkStructureType                         sType;
    const(void)                            *pNext;
    VkPipelineTessellationStateCreateFlags  flags;
    uint                                    patchControlPoints;
}

struct VkViewport {
    float  x;
    float  y;
    float  width;
    float  height;
    float  minDepth;
    float  maxDepth;
}

struct VkOffset2D {
    int  x;
    int  y;
}

struct VkExtent2D {
    uint  width;
    uint  height;
}

struct VkRect2D {
    VkOffset2D  offset;
    VkExtent2D  extent;
}

struct VkPipelineViewportStateCreateInfo {
    VkStructureType                     sType;
    const(void)                        *pNext;
    VkPipelineViewportStateCreateFlags  flags;
    uint                                viewportCount;
    const(VkViewport)                  *pViewports;
    uint                                scissorCount;
    const(VkRect2D)                    *pScissors;
}

struct VkPipelineRasterizationStateCreateInfo {
    VkStructureType                          sType;
    const(void)                             *pNext;
    VkPipelineRasterizationStateCreateFlags  flags;
    VkBool32                                 depthClampEnable;
    VkBool32                                 rasterizerDiscardEnable;
    VkPolygonMode                            polygonMode;
    VkCullModeFlags                          cullMode;
    VkFrontFace                              frontFace;
    VkBool32                                 depthBiasEnable;
    float                                    depthBiasConstantFactor;
    float                                    depthBiasClamp;
    float                                    depthBiasSlopeFactor;
    float                                    lineWidth;
}

struct VkPipelineMultisampleStateCreateInfo {
    VkStructureType                        sType;
    const(void)                           *pNext;
    VkPipelineMultisampleStateCreateFlags  flags;
    VkSampleCountFlagBits                  rasterizationSamples;
    VkBool32                               sampleShadingEnable;
    float                                  minSampleShading;
    const(VkSampleMask)                   *pSampleMask;
    VkBool32                               alphaToCoverageEnable;
    VkBool32                               alphaToOneEnable;
}

struct VkStencilOpState {
    VkStencilOp  failOp;
    VkStencilOp  passOp;
    VkStencilOp  depthFailOp;
    VkCompareOp  compareOp;
    uint         compareMask;
    uint         writeMask;
    uint         reference;
}

struct VkPipelineDepthStencilStateCreateInfo {
    VkStructureType                         sType;
    const(void)                            *pNext;
    VkPipelineDepthStencilStateCreateFlags  flags;
    VkBool32                                depthTestEnable;
    VkBool32                                depthWriteEnable;
    VkCompareOp                             depthCompareOp;
    VkBool32                                depthBoundsTestEnable;
    VkBool32                                stencilTestEnable;
    VkStencilOpState                        front;
    VkStencilOpState                        back;
    float                                   minDepthBounds;
    float                                   maxDepthBounds;
}

struct VkPipelineColorBlendAttachmentState {
    VkBool32               blendEnable;
    VkBlendFactor          srcColorBlendFactor;
    VkBlendFactor          dstColorBlendFactor;
    VkBlendOp              colorBlendOp;
    VkBlendFactor          srcAlphaBlendFactor;
    VkBlendFactor          dstAlphaBlendFactor;
    VkBlendOp              alphaBlendOp;
    VkColorComponentFlags  colorWriteMask;
}

struct VkPipelineColorBlendStateCreateInfo {
    VkStructureType                             sType;
    const(void)                                *pNext;
    VkPipelineColorBlendStateCreateFlags        flags;
    VkBool32                                    logicOpEnable;
    VkLogicOp                                   logicOp;
    uint                                        attachmentCount;
    const(VkPipelineColorBlendAttachmentState) *pAttachments;
    float[4]                                    blendConstants;
}

union VkClearColorValue {
    float[4]  float32;
    int[4]    int32;
    uint[4]   uint32;
}

struct VkClearDepthStencilValue {
    float  depth;
    uint   stencil;
}

union VkClearValue {
    VkClearColorValue         color;
    VkClearDepthStencilValue  depthStencil;
}
