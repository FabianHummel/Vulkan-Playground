// #include "vkc.h"
// #include <SDL2/SDL.h>
// #include <SDL2/SDL_vulkan.h>
// #include <stdbool.h>
// #include <stdio.h>
// #include <string.h>
// #include "stb_image.h"

// extern SDL_Window* window;

// typedef struct {
//     void* data;
//     int len;
// } FileData;

// FileData readFile(const char* path)
// {
//     FILE* f = fopen(path, "rb");
//     if (!f) {
//         SDL_Log("Could not open %s!\n", path);
//         FileData fd = { 0 };
//         return fd;
//     }
//     fseek(f, 0, SEEK_END);
//     FileData fd;
//     fd.len = (int)ftell(f);
//     fd.data = malloc(fd.len);
//     fseek(f, 0, SEEK_SET);
//     fread(fd.data, 1, fd.len, f);
//     return fd;
// }

// void createInstance(void)
// {
//     VkApplicationInfo appInfo = { 0 };
//     appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
//     appInfo.pApplicationName = "Hello Vulkan";
//     appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
//     appInfo.pEngineName = "No Engine";
//     appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
//     appInfo.apiVersion = VK_API_VERSION_1_0;

//     unsigned int extensionCount;
//     SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, NULL);
//     SDL_Log("SDL required Vulkan extensions: %d\n", extensionCount);

//     const char** ppExtensionNames = calloc(extensionCount, sizeof(const char*));
//     SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, ppExtensionNames);
//     for (unsigned i = 0; i < extensionCount; i++) {
//         SDL_Log("Extension %d: %s\n", i, ppExtensionNames[i]);
//     }

//     VkInstanceCreateInfo createInfo = { 0 };
//     createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
//     createInfo.pApplicationInfo = &appInfo;
//     createInfo.enabledLayerCount = 0;
//     createInfo.enabledExtensionCount = extensionCount;
//     createInfo.ppEnabledExtensionNames = ppExtensionNames;

//     VkResult result = vkCreateInstance(&createInfo, NULL, &vkc.instance);
//     if (result != VK_SUCCESS) {
//         SDL_Log("Error initializing Vulkan, result=%d\n", result);
//         return;
//     }

//     free(ppExtensionNames);
// }

// void findPhysicalDevice(void)
// {
//     vkc.physicalDevice = VK_NULL_HANDLE; // default value
//     uint32_t deviceCount = 0;
//     vkEnumeratePhysicalDevices(vkc.instance, &deviceCount, NULL);
//     if (deviceCount == 0) {
//         SDL_Log("Failed to find GPUs with Vulkan support!\n");
//         return;
//     }
//     SDL_Log("Found %d GPUs with Vulkan support.\n", deviceCount);

//     VkPhysicalDevice* devices = calloc(deviceCount, sizeof(VkPhysicalDevice));
//     vkEnumeratePhysicalDevices(vkc.instance, &deviceCount, devices);
//     for (unsigned i = 0; i < deviceCount; i++)
//     {
//         VkPhysicalDeviceProperties properties;
//         vkGetPhysicalDeviceProperties(devices[i], &properties);
//         SDL_Log("Device %d: %s\n", i, properties.deviceName);
//     }

//     vkc.physicalDevice = devices[0];

//     free(devices);
// }

// void createWindowSurface(void)
// {
//     SDL_bool result = SDL_Vulkan_CreateSurface(window, vkc.instance, &vkc.surface);
//     if (result == false) {
//         SDL_Log("Error creating Vulkan surface: %s\n", SDL_GetError());
//         return;
//     }
// }

// void queryQueueFamilies(void)
// {
//     vkc.graphicsQueueFamilyIndex = UINT32_MAX;
//     vkc.presentQueueFamilyIndex = UINT32_MAX;
//     uint32_t queueFamilyCount = 0;
//     vkGetPhysicalDeviceQueueFamilyProperties(vkc.physicalDevice, &queueFamilyCount, NULL);
//     VkQueueFamilyProperties* queueFamilies = NULL;
//     queueFamilies = malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
//     vkGetPhysicalDeviceQueueFamilyProperties(vkc.physicalDevice, &queueFamilyCount, queueFamilies);
//     for (unsigned i = 0; i < queueFamilyCount; i++) {
//         if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
//             vkc.graphicsQueueFamilyIndex = i;
//         }
//         VkBool32 presentSupport = VK_FALSE;
//         vkGetPhysicalDeviceSurfaceSupportKHR(vkc.physicalDevice, i, vkc.surface, &presentSupport);
//         if (presentSupport == VK_TRUE) {
//             vkc.presentQueueFamilyIndex = i;
//             if (vkc.graphicsQueueFamilyIndex == i) break; // queue that can do both
//         }
//     }
//     if (vkc.graphicsQueueFamilyIndex == UINT32_MAX) {
//         SDL_Log("Could not find queue index with VK_QUEUE_GRAPHICS_BIT!\n");
//         return;
//     }

//     if (vkc.presentQueueFamilyIndex == UINT32_MAX) {
//         SDL_Log("Could not find queue index with surface present support!\n");
//         return;
//     }
//     free(queueFamilies);
// }

// void createLogicalDevice(void)
// {
//     VkDeviceQueueCreateInfo queueCreateInfo = { 0 };
//     float queuePriority = 1.0f;
//     queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//     queueCreateInfo.queueFamilyIndex = vkc.graphicsQueueFamilyIndex;
//     queueCreateInfo.queueCount = 1;
//     queueCreateInfo.pQueuePriorities = &queuePriority;

//     VkPhysicalDeviceFeatures deviceFeatures = { 0 };
//     deviceFeatures.samplerAnisotropy = VK_TRUE;

//     VkDeviceCreateInfo createInfo = { 0 };
//     createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
//     createInfo.pQueueCreateInfos = &queueCreateInfo;
//     createInfo.queueCreateInfoCount = 1;
//     createInfo.pEnabledFeatures = &deviceFeatures;
//     const char* extensionNames[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
//     createInfo.ppEnabledExtensionNames = extensionNames;
//     createInfo.enabledExtensionCount = 1;
//     createInfo.enabledLayerCount = 0;

//     VkResult result = vkCreateDevice(vkc.physicalDevice, &createInfo, NULL, &vkc.device);
//     if (result != VK_SUCCESS) {
//         SDL_Log("Error creating logical Vulkan device: %d\n", result);
//     }
// }

// void getQueues(void)
// {
//     vkGetDeviceQueue(vkc.device, vkc.graphicsQueueFamilyIndex, 0, &vkc.graphicsQueue);
//     vkGetDeviceQueue(vkc.device, vkc.presentQueueFamilyIndex, 0, &vkc.presentQueue);
// }

// void getSurfaceFormats(void)
// {
//     uint32_t formatCount;
//     vkGetPhysicalDeviceSurfaceFormatsKHR(vkc.physicalDevice, vkc.surface, &formatCount, NULL);

//     VkSurfaceFormatKHR* formats = NULL;
//     if (formatCount != 0)
//     {
//         formats = calloc(formatCount, sizeof(VkSurfaceFormatKHR));
//         vkGetPhysicalDeviceSurfaceFormatsKHR(vkc.physicalDevice, vkc.surface, &formatCount, formats);
//         vkc.surfaceFormat = formats[0]; // default format
//         for (unsigned i = 0; i < formatCount; i++)
//         {
//             if (formats[i].format == VK_FORMAT_R8G8B8A8_SRGB &&
//                 formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
//             {
//                 SDL_Log("Surface VK_FORMAT_R8G8B8A8_SRGB VK_COLORSPACE_SRGB_NONLINEAR_KHR.\n");
//                 vkc.surfaceFormat = formats[i];
//                 break;
//             }

//             if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
//                 formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
//             {
//                 SDL_Log("Surface format VK_FORMAT_B8G8R8A8_SRGB VK_COLORSPACE_SRGB_NONLINEAR_KHR.\n");
//                 vkc.surfaceFormat = formats[i];
//                 break;
//             }
//         }
//         free(formats);
//     }
//     else
//     {
//         free(formats);
//         SDL_Log("No valid surface formats found!\n");
//         return;
//     }

//     uint32_t presentModeCount;
//     vkGetPhysicalDeviceSurfacePresentModesKHR(vkc.physicalDevice, vkc.surface, &presentModeCount, NULL);
//     VkPresentModeKHR* presentModes = NULL;
//     if (presentModeCount != 0)
//     {
//         presentModes = calloc(presentModeCount, sizeof(VkPresentModeKHR));
//         vkGetPhysicalDeviceSurfacePresentModesKHR(vkc.physicalDevice, vkc.surface, &presentModeCount, presentModes);
//         vkc.presentMode = presentModes[0]; // default present mode
//         for (unsigned i = 0; i < presentModeCount; i++)
//         {
//             if (presentModes[i] == VK_PRESENT_MODE_FIFO_KHR) {
//                 SDL_Log("Present mode FIFO.\n");
//                 vkc.presentMode = presentModes[i];
//                 break;
//             }
//         }
//     }
//     else
//     {
//         SDL_Log("No valid present modes found!\n");
//         return;
//     }

//     vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkc.physicalDevice, vkc.surface, &vkc.surfaceCapabilities);
//     if (vkc.surfaceCapabilities.currentExtent.width != UINT32_MAX)
//     {
//         vkc.swapChainExtent = vkc.surfaceCapabilities.currentExtent;
//     }
//     else
//     {
//         int w, h;
//         SDL_GetWindowSizeInPixels(window, &w, &h);
//         VkExtent2D minExtent = vkc.surfaceCapabilities.minImageExtent;
//         VkExtent2D maxExtent = vkc.surfaceCapabilities.maxImageExtent;
//         vkc.swapChainExtent.width = SDL_clamp((uint32_t)w, minExtent.width, maxExtent.width);
//         vkc.swapChainExtent.height = SDL_clamp((uint32_t)h, minExtent.height, maxExtent.height);
//     }
//     SDL_Log("Swap extent: %dx%d", vkc.swapChainExtent.width, vkc.swapChainExtent.height);
// }

// void createSwapchain(void)
// {
//     uint32_t imageCount = vkc.surfaceCapabilities.minImageCount + 1;
//     if (imageCount > vkc.surfaceCapabilities.maxImageCount && vkc.surfaceCapabilities.maxImageCount > 0) {
//         imageCount = vkc.surfaceCapabilities.maxImageCount;
//     }
//     VkSwapchainCreateInfoKHR createInfo = { 0 };
//     createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
//     createInfo.surface = vkc.surface;
//     createInfo.minImageCount = imageCount;
//     createInfo.imageFormat = vkc.surfaceFormat.format;
//     createInfo.imageColorSpace = vkc.surfaceFormat.colorSpace;
//     createInfo.imageExtent = vkc.swapChainExtent;
//     createInfo.imageArrayLayers = 1;
//     createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
//     createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
//     createInfo.preTransform = vkc.surfaceCapabilities.currentTransform;
//     createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
//     createInfo.presentMode = vkc.presentMode;
//     createInfo.clipped = VK_TRUE;
//     createInfo.oldSwapchain = VK_NULL_HANDLE;

//     VkResult result = vkCreateSwapchainKHR(vkc.device, &createInfo, NULL, &vkc.swapChain);
//     if (result != VK_SUCCESS) {
//         SDL_Log("Could not create swapchain: %d\n", result);
//         return;
//     }
// }

// VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
//     VkImageViewCreateInfo viewInfo = { 0 };
//     viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//     viewInfo.image = image;
//     viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//     viewInfo.format = format;
//     viewInfo.subresourceRange.aspectMask = aspectFlags;
//     viewInfo.subresourceRange.baseMipLevel = 0;
//     viewInfo.subresourceRange.levelCount = 1;
//     viewInfo.subresourceRange.baseArrayLayer = 0;
//     viewInfo.subresourceRange.layerCount = 1;

//     VkImageView imageView;
//     if (vkCreateImageView(vkc.device, &viewInfo, NULL, &imageView) != VK_SUCCESS) {
//         SDL_Log("failed to create image view!\n");
//         return (VkImageView) 0;
//     }

//     return imageView;
// }

// void createSwapchainImageViews(void)
// {
//     vkGetSwapchainImagesKHR(vkc.device, vkc.swapChain, &vkc.swapChainImageCount, NULL);
//     vkc.swapChainImages = calloc(vkc.swapChainImageCount, sizeof(VkImage));
//     vkGetSwapchainImagesKHR(vkc.device, vkc.swapChain, &vkc.swapChainImageCount, vkc.swapChainImages);
//     SDL_Log("Swapchain has %d images.\n", vkc.swapChainImageCount);

//     vkc.swapChainImageViews = calloc(vkc.swapChainImageCount, sizeof(VkImageView));

//     for (int i = 0; i < vkc.swapChainImageCount; i++)
//     {
//         vkc.swapChainImageViews[i] = createImageView(vkc.swapChainImages[i], vkc.surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
//     }
// }

// void createCommandPool(void)
// {
//     VkCommandPoolCreateInfo poolInfo = { 0 };
//     poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//     poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
//     poolInfo.queueFamilyIndex = vkc.graphicsQueueFamilyIndex;
//     VkResult result = vkCreateCommandPool(vkc.device, &poolInfo, NULL, &vkc.commandPool);
//     if (result != VK_SUCCESS) {
//         SDL_Log("Could not create command pool: %d\n", result);
//         return;
//     }
// }

// uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
//     VkPhysicalDeviceMemoryProperties memProperties;
//     vkGetPhysicalDeviceMemoryProperties(vkc.physicalDevice, &memProperties);
//     for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
//         if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
//             return i;
//         }
//     }

//     SDL_Log("failed to find suitable memory type!");
//     return UINT32_MAX;
// }

// void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory) {
//     VkBufferCreateInfo bufferInfo = { 0 };
//     bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//     bufferInfo.size = size;
//     bufferInfo.usage = usage;
//     bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

//     if (vkCreateBuffer(vkc.device, &bufferInfo, NULL, buffer) != VK_SUCCESS) {
//         SDL_Log("failed to create buffer!");
//         return;
//     }

//     VkMemoryRequirements memRequirements;
//     vkGetBufferMemoryRequirements(vkc.device, *buffer, &memRequirements);

//     VkMemoryAllocateInfo allocInfo = { 0 };
//     allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//     allocInfo.allocationSize = memRequirements.size;
//     allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

//     if (vkAllocateMemory(vkc.device, &allocInfo, NULL, bufferMemory) != VK_SUCCESS) {
//         SDL_Log("failed to allocate buffer memory!");
//         return;
//     }

//     vkBindBufferMemory(vkc.device, *buffer, *bufferMemory, 0);
// }

// void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
//     VkMemoryPropertyFlags memoryProperties, VkImage *image, VkDeviceMemory *imageMemory)
// {
//     VkImageCreateInfo imageInfo = { 0 };
//     imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//     imageInfo.imageType = VK_IMAGE_TYPE_2D;
//     imageInfo.extent.width = width;
//     imageInfo.extent.height = height;
//     imageInfo.extent.depth = 1;
//     imageInfo.mipLevels = 1;
//     imageInfo.arrayLayers = 1;
//     imageInfo.format = format;
//     imageInfo.tiling = tiling;
//     imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//     imageInfo.usage = usage;
//     imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//     imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//     imageInfo.flags = 0; // Optional

//     if (vkCreateImage(vkc.device, &imageInfo, NULL, image) != VK_SUCCESS) {
//         SDL_Log("failed to create image!\n");
//     }

//     VkMemoryRequirements memRequirements;
//     vkGetImageMemoryRequirements(vkc.device, *image, &memRequirements);

//     VkMemoryAllocateInfo allocInfo = {0};
//     allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//     allocInfo.allocationSize = memRequirements.size;
//     allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, memoryProperties);

//     if (vkAllocateMemory(vkc.device, &allocInfo, NULL, imageMemory) != VK_SUCCESS) {
//         SDL_Log("failed to allocate image memory!\n");
//     }

//     vkBindImageMemory(vkc.device, *image, *imageMemory, 0);
// }

// VkCommandBuffer beginSingleTimeCommands(void)
// {
//     VkCommandBufferAllocateInfo allocInfo = { 0 };
//     allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//     allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//     allocInfo.commandPool = vkc.commandPool;
//     allocInfo.commandBufferCount = 1;

//     VkCommandBuffer commandBuffer;
//     vkAllocateCommandBuffers(vkc.device, &allocInfo, &commandBuffer);

//     VkCommandBufferBeginInfo beginInfo = { 0 };
//     beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//     beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

//     vkBeginCommandBuffer(commandBuffer, &beginInfo);

//     return commandBuffer;
// }

// void endSingleTimeCommands(VkCommandBuffer commandBuffer)
// {
//     vkEndCommandBuffer(commandBuffer);

//     VkSubmitInfo submitInfo = { 0 };
//     submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//     submitInfo.commandBufferCount = 1;
//     submitInfo.pCommandBuffers = &commandBuffer;

//     vkQueueSubmit(vkc.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
//     vkQueueWaitIdle(vkc.graphicsQueue);

//     vkFreeCommandBuffers(vkc.device, vkc.commandPool, 1, &commandBuffer);
// }

// void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
// {
//     VkCommandBuffer commandBuffer = beginSingleTimeCommands();

//     VkImageMemoryBarrier barrier = { 0 };
//     barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//     barrier.oldLayout = oldLayout;
//     barrier.newLayout = newLayout;

//     barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//     barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

//     barrier.image = image;
//     barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//     barrier.subresourceRange.baseMipLevel = 0;
//     barrier.subresourceRange.levelCount = 1;
//     barrier.subresourceRange.baseArrayLayer = 0;
//     barrier.subresourceRange.layerCount = 1;

//     VkPipelineStageFlags sourceStage;
//     VkPipelineStageFlags destinationStage;

//     if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
//         barrier.srcAccessMask = 0;
//         barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

//         sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//         destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//     }
//     else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
//         barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//         barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

//         sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//         destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//     }
//     else {
//         SDL_Log("unsupported layout transition!\n");
//     }

//     vkCmdPipelineBarrier(
//         commandBuffer,
//         sourceStage, destinationStage,
//         0,
//         0, NULL,
//         0, NULL,
//         1, &barrier
//     );

//     endSingleTimeCommands(commandBuffer);
// }

// void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
// {
//     VkCommandBuffer commandBuffer = beginSingleTimeCommands();

//     VkBufferImageCopy region = { 0 };
//     region.bufferOffset = 0;
//     region.bufferRowLength = 0;
//     region.bufferImageHeight = 0;

//     region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//     region.imageSubresource.mipLevel = 0;
//     region.imageSubresource.baseArrayLayer = 0;
//     region.imageSubresource.layerCount = 1;

//     region.imageOffset.x = 0;
//     region.imageOffset.y = 0;
//     region.imageOffset.z = 0;

//     region.imageExtent.width = width;
//     region.imageExtent.height = height;
//     region.imageExtent.depth = 1;

//     vkCmdCopyBufferToImage(
//         commandBuffer,
//         buffer,
//         image,
//         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//         1,
//         &region
//     );

//     endSingleTimeCommands(commandBuffer);
// }

// bool hasStencilComponent(VkFormat format) 
// {
//     return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
// }

// VkFormat findSupportedFormat(const VkFormat* candidates, size_t candidatesCount, VkImageTiling tiling, VkFormatFeatureFlags features)
// {
//     for (int i = 0; i < candidatesCount; i++)
//     {
//         VkFormatProperties props;
//         vkGetPhysicalDeviceFormatProperties(vkc.physicalDevice, candidates[i], &props);
//         if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
//             return candidates[i];
//         }
//         else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
//             return candidates[i];
//         }
//     }
//     SDL_Log("failed to find supported format!\n");
//     return (VkFormat) 0;
// }

// VkFormat findDepthFormat(void)
// {
//     VkFormat candidates[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
//     size_t candidatesCount = sizeof(candidates) / sizeof(candidates[0]);
//     return findSupportedFormat(candidates, candidatesCount, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
// }

// void createDepthResources(void)
// {
//     VkFormat depthFormat = findDepthFormat();
//     createImage(vkc.swapChainExtent.width, vkc.swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vkc.depthImage, &vkc.depthImageMemory);
//     vkc.depthImageView = createImageView(vkc.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
// }

// void createOffscreenTexture(void)
// {
//     createImage(vkc.swapChainExtent.width, vkc.swapChainExtent.height, vkc.surfaceFormat.format, VK_IMAGE_TILING_OPTIMAL,
//         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//         &vkc.offscreenTexture.image, &vkc.offscreenTexture.deviceMemory);
//     vkc.offscreenTexture.imageView = createImageView(vkc.offscreenTexture.image, vkc.surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
// }

// void createTextureSampler(void)
// {
//     VkSamplerCreateInfo samplerInfo = { 0 };
//     samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
//     samplerInfo.magFilter = VK_FILTER_LINEAR;
//     samplerInfo.minFilter = VK_FILTER_LINEAR;
//     samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//     samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//     samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//     samplerInfo.anisotropyEnable = VK_TRUE;

//     VkPhysicalDeviceProperties properties = {0};
//     vkGetPhysicalDeviceProperties(vkc.physicalDevice, &properties);
//     samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
//     samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
//     samplerInfo.unnormalizedCoordinates = VK_FALSE;
//     samplerInfo.compareEnable = VK_FALSE;
//     samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
//     samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
//     samplerInfo.mipLodBias = 0.0f;
//     samplerInfo.minLod = 0.0f;
//     samplerInfo.maxLod = 0.0f;

//     if (vkCreateSampler(vkc.device, &samplerInfo, NULL, &vkc.textureSampler) != VK_SUCCESS) {
//         SDL_Log("failed to create texture sampler!\n");
//     }
// }

// void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
// {
//     VkCommandBuffer commandBuffer = beginSingleTimeCommands();

//     VkBufferCopy copyRegion = {0};
//     copyRegion.srcOffset = 0; // Optional
//     copyRegion.dstOffset = 0; // Optional
//     copyRegion.size = size;
//     vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

//     endSingleTimeCommands(commandBuffer);
// }

// void createUniformBuffers(void)
// {
//     VkDeviceSize bufferSize = sizeof(UniformBufferObject);
//     createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vkc.uniformBuffers.vsShared.buffer, &vkc.uniformBuffers.vsShared.deviceMemory);
//     vkMapMemory(vkc.device, vkc.uniformBuffers.vsShared.deviceMemory, 0, bufferSize, 0, &vkc.uniformBuffers.vsShared.mappedMemory);

//     bufferSize = sizeof(PostProcUBO);
//     createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vkc.uniformBuffers.vsQuad.buffer, &vkc.uniformBuffers.vsQuad.deviceMemory);
//     vkMapMemory(vkc.device, vkc.uniformBuffers.vsQuad.deviceMemory, 0, bufferSize, 0, &vkc.uniformBuffers.vsQuad.mappedMemory);
// }

// void allocateCommandBuffers(void)
// {
//     VkCommandBufferAllocateInfo allocInfo = { 0 };
//     allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//     allocInfo.commandPool = vkc.commandPool;
//     allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//     allocInfo.commandBufferCount = vkc.swapChainImageCount;

//     vkc.commandBuffers = calloc(allocInfo.commandBufferCount, sizeof(VkCommandBuffer));

//     VkResult result = vkAllocateCommandBuffers(vkc.device, &allocInfo, vkc.commandBuffers);
//     if (result != VK_SUCCESS) {
//         SDL_Log("Failed to allocate command buffer: %d\n", result);
//         return;
//     }
// }

// void createSyncObjects(void)
// {
//     VkSemaphoreCreateInfo semaphoreInfo = { 0 };
//     semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

//     VkFenceCreateInfo fenceInfo = { 0 };
//     fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//     fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // start in signaled state

//     vkc.imageAvailableSemaphores = calloc(vkc.swapChainImageCount, sizeof(VkSemaphore));
//     vkc.renderFinishedSemaphores = calloc(vkc.swapChainImageCount, sizeof(VkSemaphore));
//     vkc.waitFences = calloc(vkc.swapChainImageCount, sizeof(VkFence));
//     for (int i = 0; i < vkc.swapChainImageCount; i++) {
//         if (vkCreateSemaphore(vkc.device, &semaphoreInfo, NULL, &vkc.imageAvailableSemaphores[i]) != VK_SUCCESS ||
//             vkCreateSemaphore(vkc.device, &semaphoreInfo, NULL, &vkc.renderFinishedSemaphores[i]) != VK_SUCCESS) {
//             SDL_Log("Failed to create semaphore!\n");
//         }
//         if (vkCreateFence(vkc.device, &fenceInfo, NULL, &vkc.waitFences[i]) != VK_SUCCESS)
//         {
//             SDL_Log("Failed to create fence!\n");
//         }
//     }
// }

// void createDescriptorSetLayouts(void)
// {
//     VkDescriptorSetLayoutBinding uboLayoutBinding = { 0 };
//     uboLayoutBinding.binding = 0;
//     uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//     uboLayoutBinding.descriptorCount = 1;
//     uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
//     uboLayoutBinding.pImmutableSamplers = NULL; // Optional

//     VkDescriptorSetLayoutBinding samplerLayoutBinding = { 0 };
//     samplerLayoutBinding.binding = 1;
//     samplerLayoutBinding.descriptorCount = 1;
//     samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//     samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
//     samplerLayoutBinding.pImmutableSamplers = NULL; // Optional

//     VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
//     layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//     VkDescriptorSetLayoutBinding bindings[] = { uboLayoutBinding, samplerLayoutBinding };
//     layoutInfo.bindingCount = sizeof(bindings) / sizeof(bindings[0]);
//     layoutInfo.pBindings = bindings;

//     if (vkCreateDescriptorSetLayout(vkc.device, &layoutInfo, NULL, &vkc.descriptorSetLayouts.textured) != VK_SUCCESS) {
//         SDL_Log("failed to create descriptor set layout!");
//         return;
//     }
// }


// void createDescriptorPool(void)
// {
//     VkDescriptorPoolSize poolSize = { 0 };
//     poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//     poolSize.descriptorCount = 2;

//     size_t poolSizesCount = 2;
//     VkDescriptorPoolSize *poolSizes = calloc(poolSizesCount, sizeof(VkDescriptorPoolSize));
//     poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//     poolSizes[0].descriptorCount = 2;
//     poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//     poolSizes[1].descriptorCount = 2;

//     VkDescriptorPoolCreateInfo poolInfo = { 0 };
//     poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//     poolInfo.poolSizeCount = (uint32_t) poolSizesCount;
//     poolInfo.pPoolSizes = poolSizes;
//     poolInfo.maxSets = 2;

//     if (vkCreateDescriptorPool(vkc.device, &poolInfo, NULL, &vkc.descriptorPool) != VK_SUCCESS) {
//         SDL_Log("failed to create descriptor pool!");
//         free(poolSizes);
//         return;
//     }
//     free(poolSizes);
// }

// void createDescriptorSets(void)
// {
//     VkDescriptorSetAllocateInfo allocInfo = {0};
//     allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//     allocInfo.descriptorPool = vkc.descriptorPool;
//     allocInfo.descriptorSetCount = 1;
//     allocInfo.pSetLayouts = &vkc.descriptorSetLayouts.textured;

//     if (vkAllocateDescriptorSets(vkc.device, &allocInfo, &vkc.descriptorSets.quad) != VK_SUCCESS) {
//         SDL_Log("failed to allocate descriptor set (quad)!");
//         return;
//     }

//     {
//         VkDescriptorBufferInfo bufferInfo = { 0 };
//         bufferInfo.buffer = vkc.uniformBuffers.vsQuad.buffer;
//         bufferInfo.offset = 0;
//         bufferInfo.range = sizeof(PostProcUBO);

//         VkDescriptorImageInfo imageInfo = { 0 };
//         imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//         imageInfo.imageView = vkc.offscreenTexture.imageView;
//         imageInfo.sampler = vkc.textureSampler;

//         size_t descriptorWritesCount = 2;
//         VkWriteDescriptorSet *descriptorWrites = calloc(descriptorWritesCount, sizeof(VkWriteDescriptorSet));

//         descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//         descriptorWrites[0].dstSet = vkc.descriptorSets.quad;
//         descriptorWrites[0].dstBinding = 0;
//         descriptorWrites[0].dstArrayElement = 0;
//         descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//         descriptorWrites[0].descriptorCount = 1;
//         descriptorWrites[0].pBufferInfo = &bufferInfo;

//         descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//         descriptorWrites[1].dstSet = vkc.descriptorSets.quad;
//         descriptorWrites[1].dstBinding = 1;
//         descriptorWrites[1].dstArrayElement = 0;
//         descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//         descriptorWrites[1].descriptorCount = 1;
//         descriptorWrites[1].pImageInfo = &imageInfo;

//         vkUpdateDescriptorSets(vkc.device, (uint32_t) descriptorWritesCount, descriptorWrites, 0, NULL);
//         free(descriptorWrites);
//     }
// }

// VkShaderModule createShaderModule(FileData fd)
// {
//     VkShaderModuleCreateInfo createInfo = { 0 };
//     createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
//     createInfo.codeSize = fd.len;
//     createInfo.pCode = fd.data;
//     VkShaderModule shaderModule;
//     VkResult result = vkCreateShaderModule(vkc.device, &createInfo, NULL, &shaderModule);
//     if (result != VK_SUCCESS) {
//         SDL_Log("vkCreateShaderModule: %d\n", result);
//     }
//     return shaderModule;
// }

// void createPipelineCache(void)
// {
//     VkPipelineCacheCreateInfo pipelineCacheCreateInfo = { 0 };
//     pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
//     if (vkCreatePipelineCache(vkc.device, &pipelineCacheCreateInfo, NULL, &vkc.pipelineCache) != VK_SUCCESS) {
//         SDL_Log("failed to create pipeline cache\n");
//         return;
//     }
// }

// void createGraphicsPipelines(void)
// {
//     FileData vertShaderCodeShaded = readFile("shader.vert.spv");
//     FileData fragShaderCodeShaded = readFile("shader.frag.spv");
//     VkShaderModule vertShaderModuleShaded = createShaderModule(vertShaderCodeShaded);
//     VkShaderModule fragShaderModuleShaded = createShaderModule(fragShaderCodeShaded);

//     FileData vertShaderCodeQuad = readFile("quad.vert.spv");
//     FileData fragShaderCodeQuad = readFile("quad.frag.spv");
//     VkShaderModule vertShaderModuleQuad = createShaderModule(vertShaderCodeQuad);
//     VkShaderModule fragShaderModuleQuad = createShaderModule(fragShaderCodeQuad);

//     VkPipelineShaderStageCreateInfo vertShaderStageInfo = { 0 };
//     vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//     vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
//     vertShaderStageInfo.module = vertShaderModuleShaded;
//     vertShaderStageInfo.pName = "main";

//     VkPipelineShaderStageCreateInfo fragShaderStageInfo = { 0 };
//     fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//     fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//     fragShaderStageInfo.module = fragShaderModuleShaded;
//     fragShaderStageInfo.pName = "main";

//     VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

//     VkDynamicState dynamicStates[] = {
//         VK_DYNAMIC_STATE_VIEWPORT,
//         VK_DYNAMIC_STATE_SCISSOR
//     };

//     VkPipelineDynamicStateCreateInfo dynamicState = { 0 };
//     dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
//     dynamicState.dynamicStateCount = 2;
//     dynamicState.pDynamicStates = dynamicStates;

//     VkVertexInputBindingDescription bindingDescription = { 0 };
//     bindingDescription.binding = 0;
//     bindingDescription.stride = sizeof(Vertex);
//     bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

//     VkVertexInputAttributeDescription attributeDescriptions[4] = { 0 };
//     const int attributeDescriptionsCount = sizeof(attributeDescriptions) / sizeof(attributeDescriptions[0]);

//     attributeDescriptions[0].binding = 0;
//     attributeDescriptions[0].location = 0;
//     attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//     attributeDescriptions[0].offset = offsetof(Vertex, pos);

//     attributeDescriptions[1].binding = 0;
//     attributeDescriptions[1].location = 1;
//     attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//     attributeDescriptions[1].offset = offsetof(Vertex, color);

//     attributeDescriptions[2].binding = 0;
//     attributeDescriptions[2].location = 2;
//     attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
//     attributeDescriptions[2].offset = offsetof(Vertex, normal);

//     attributeDescriptions[3].binding = 0;
//     attributeDescriptions[3].location = 3;
//     attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
//     attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

//     VkPipelineVertexInputStateCreateInfo vertexInputInfo = { 0 };
//     vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//     vertexInputInfo.vertexBindingDescriptionCount = 1;
//     vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
//     vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptionsCount;
//     vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

//     VkPipelineInputAssemblyStateCreateInfo inputAssembly = { 0 };
//     inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//     inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//     inputAssembly.primitiveRestartEnable = VK_FALSE;

//     VkViewport viewport = { 0 };
//     viewport.x = 0.0f;
//     viewport.y = 0.0f;
//     viewport.width = (float)vkc.swapChainExtent.width;
//     viewport.height = (float)vkc.swapChainExtent.height;
//     viewport.minDepth = 0.0f;
//     viewport.maxDepth = 1.0f;

//     VkRect2D scissor = { 0 };
//     scissor.offset.x = 0;
//     scissor.offset.y = 0;
//     scissor.extent = vkc.swapChainExtent;

//     VkPipelineViewportStateCreateInfo viewportState = { 0 };
//     viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//     viewportState.viewportCount = 1;
//     viewportState.pViewports = &viewport;
//     viewportState.scissorCount = 1;
//     viewportState.pScissors = &scissor;

//     VkPipelineRasterizationStateCreateInfo rasterizer = { 0 };
//     rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//     rasterizer.depthClampEnable = VK_FALSE;
//     rasterizer.rasterizerDiscardEnable = VK_FALSE;
//     rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
//     rasterizer.lineWidth = 1.0f;
//     rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
//     rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
//     rasterizer.depthBiasEnable = VK_FALSE;
//     rasterizer.depthBiasConstantFactor = 0.0f; // Optional
//     rasterizer.depthBiasClamp = 0.0f; // Optional
//     rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

//     VkPipelineMultisampleStateCreateInfo multisampling = { 0 };
//     multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//     multisampling.sampleShadingEnable = VK_FALSE;
//     multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
//     multisampling.minSampleShading = 1.0f; // Optional
//     multisampling.pSampleMask = NULL; // Optional
//     multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
//     multisampling.alphaToOneEnable = VK_FALSE; // Optional

//     VkPipelineColorBlendAttachmentState colorBlendAttachment = { 0 };
//     colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//     colorBlendAttachment.blendEnable = VK_FALSE;
//     colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
//     colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
//     colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
//     colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
//     colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
//     colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

//     VkPipelineColorBlendStateCreateInfo colorBlending = { 0 };
//     colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//     colorBlending.logicOpEnable = VK_FALSE;
//     colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
//     colorBlending.attachmentCount = 1;
//     colorBlending.pAttachments = &colorBlendAttachment;
//     colorBlending.blendConstants[0] = 0.0f; // Optional
//     colorBlending.blendConstants[1] = 0.0f; // Optional
//     colorBlending.blendConstants[2] = 0.0f; // Optional
//     colorBlending.blendConstants[3] = 0.0f; // Optional

//     VkPipelineDepthStencilStateCreateInfo depthStencil = { 0 };
//     depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
//     depthStencil.depthTestEnable = VK_TRUE;
//     depthStencil.depthWriteEnable = VK_TRUE;
//     depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
//     depthStencil.depthBoundsTestEnable = VK_FALSE;
//     depthStencil.minDepthBounds = 0.0f; // Optional
//     depthStencil.maxDepthBounds = 1.0f; // Optional
//     depthStencil.stencilTestEnable = VK_FALSE;
//     VkStencilOpState emptyStencilOpState = { 0 };
//     depthStencil.front = emptyStencilOpState; // Optional
//     depthStencil.back = emptyStencilOpState; // Optional

//     VkPipelineLayoutCreateInfo pipelineLayoutInfo = { 0 };
//     pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//     pipelineLayoutInfo.setLayoutCount = 1;
//     pipelineLayoutInfo.pSetLayouts = &vkc.descriptorSetLayouts.textured;
//     pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
//     pipelineLayoutInfo.pPushConstantRanges = NULL; // Optional

//     VkResult result = vkCreatePipelineLayout(vkc.device, &pipelineLayoutInfo, NULL, &vkc.pipelineLayouts.shadedOffscreen);
//     if (result != VK_SUCCESS) {
//         SDL_Log("failed to create pipeline layout (offscreen)!");
//     }
//     result = vkCreatePipelineLayout(vkc.device, &pipelineLayoutInfo, NULL, &vkc.pipelineLayouts.quad);
//     if (result != VK_SUCCESS) {
//         SDL_Log("failed to create pipeline layout (quad)!");
//     }

//     VkGraphicsPipelineCreateInfo pipelineInfo = { 0 };
//     pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//     pipelineInfo.stageCount = 2;
//     pipelineInfo.pStages = shaderStages;
//     pipelineInfo.pVertexInputState = &vertexInputInfo;
//     pipelineInfo.pInputAssemblyState = &inputAssembly;
//     pipelineInfo.pViewportState = &viewportState;
//     pipelineInfo.pRasterizationState = &rasterizer;
//     pipelineInfo.pMultisampleState = &multisampling;
//     pipelineInfo.pDepthStencilState = &depthStencil;
//     pipelineInfo.pColorBlendState = &colorBlending;
//     pipelineInfo.pDynamicState = &dynamicState;
//     pipelineInfo.layout = vkc.pipelineLayouts.quad;
//     pipelineInfo.renderPass = vkc.offscreenRenderPass;
//     pipelineInfo.subpass = 0;
//     pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
//     pipelineInfo.basePipelineIndex = -1; // Optional

//     result = vkCreateGraphicsPipelines(vkc.device, vkc.pipelineCache, 1, &pipelineInfo, NULL, &vkc.pipelines.shadedOffscreen);
//     if (result != VK_SUCCESS) {
//         SDL_Log("failed to create graphics pipeline (offscreen pass)!");
//     }

//     shaderStages[0].module = vertShaderModuleQuad;
//     shaderStages[1].module = fragShaderModuleQuad;
//     vertexInputInfo.vertexBindingDescriptionCount = 0;
//     vertexInputInfo.pVertexBindingDescriptions = NULL;
//     vertexInputInfo.vertexAttributeDescriptionCount = 0;
//     vertexInputInfo.pVertexAttributeDescriptions = NULL;
//     pipelineInfo.renderPass = vkc.renderPass;
//     rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT; // triangle in quad.vert faces backwards

//     result = vkCreateGraphicsPipelines(vkc.device, vkc.pipelineCache, 1, &pipelineInfo, NULL, &vkc.pipelines.quad);
//     if (result != VK_SUCCESS) {
//         SDL_Log("failed to create graphics pipeline (quad pass)!");
//     }

//     vkDestroyShaderModule(vkc.device, fragShaderModuleShaded, NULL);
//     vkDestroyShaderModule(vkc.device, vertShaderModuleShaded, NULL);
//     free(vertShaderCodeShaded.data);
//     free(fragShaderCodeShaded.data);
//     vkDestroyShaderModule(vkc.device, fragShaderModuleQuad, NULL);
//     vkDestroyShaderModule(vkc.device, vertShaderModuleQuad, NULL);
//     free(vertShaderCodeQuad.data);
//     free(fragShaderCodeQuad.data);
// }

// void createRenderPass(void)
// {
//     VkAttachmentDescription colorAttachment = { 0 };
//     colorAttachment.format = vkc.surfaceFormat.format;
//     colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//     colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//     colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//     colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//     colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//     colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//     colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

//     VkAttachmentReference colorAttachmentRef = { 0 };
//     colorAttachmentRef.attachment = 0;
//     colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

//     VkAttachmentDescription depthAttachment = { 0 };
//     depthAttachment.format = findDepthFormat();
//     depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//     depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//     depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//     depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//     depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//     depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//     depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

//     VkAttachmentReference depthAttachmentRef = { 0 };
//     depthAttachmentRef.attachment = 1;
//     depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

//     VkSubpassDescription subpass = { 0 };
//     subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//     subpass.colorAttachmentCount = 1;
//     subpass.pColorAttachments = &colorAttachmentRef;
//     subpass.pDepthStencilAttachment = &depthAttachmentRef;

//     VkSubpassDependency dependencies[2] = {0};
//     dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
//     dependencies[0].dstSubpass = 0;
//     dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//     dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//     dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
//     dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//     dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

//     dependencies[1].srcSubpass = 0;
//     dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
//     dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//     dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//     dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//     dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
//     dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

//     VkRenderPassCreateInfo renderPassInfo = { 0 };
//     renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//     VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };
//     renderPassInfo.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
//     renderPassInfo.pAttachments = attachments;
//     renderPassInfo.subpassCount = 1;
//     renderPassInfo.pSubpasses = &subpass;
//     renderPassInfo.dependencyCount = 2;
//     renderPassInfo.pDependencies = dependencies;

//     VkResult result = vkCreateRenderPass(vkc.device, &renderPassInfo, NULL, &vkc.offscreenRenderPass);
//     if (result != VK_SUCCESS) {
//         SDL_Log("failed to create render pass (offscreen)!\n");
//     }


//     attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

//     dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
//     dependencies[0].dstSubpass = 0;
//     dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
//     dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
//     dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//     dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
//     dependencies[0].dependencyFlags = 0;

//     dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
//     dependencies[1].dstSubpass = 0;
//     dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//     dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//     dependencies[1].srcAccessMask = 0;
//     dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
//     dependencies[1].dependencyFlags = 0;

//     result = vkCreateRenderPass(vkc.device, &renderPassInfo, NULL, &vkc.renderPass);
//     if (result != VK_SUCCESS) {
//         SDL_Log("failed to create render pass (to screen)!\n");
//     }
// }

// void createFramebuffers(void) {
//     // one framebuffer for every swap chain image
//     vkc.swapChainFramebuffers = calloc(vkc.swapChainImageCount, sizeof(VkFramebuffer));
//     for (size_t i = 0; i < vkc.swapChainImageCount; i++) {
//         VkImageView attachments[] = {
//             vkc.swapChainImageViews[i],
//             vkc.depthImageView,
//         };

//         VkFramebufferCreateInfo framebufferInfo = { 0 };
//         framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//         framebufferInfo.renderPass = vkc.renderPass;
//         framebufferInfo.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
//         framebufferInfo.pAttachments = attachments;
//         framebufferInfo.width = vkc.swapChainExtent.width;
//         framebufferInfo.height = vkc.swapChainExtent.height;
//         framebufferInfo.layers = 1;

//         if (vkCreateFramebuffer(vkc.device, &framebufferInfo, NULL, &vkc.swapChainFramebuffers[i]) != VK_SUCCESS) {
//             SDL_Log("failed to create framebuffer!");
//             return;
//         }
//     }

//     // framebuffer for offscreen render target
//     {
//         VkImageView attachments[] = {
//             vkc.offscreenTexture.imageView,
//             vkc.depthImageView,
//         };

//         VkFramebufferCreateInfo framebufferInfo = { 0 };
//         framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//         framebufferInfo.renderPass = vkc.offscreenRenderPass;
//         framebufferInfo.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
//         framebufferInfo.pAttachments = attachments;
//         framebufferInfo.width = vkc.swapChainExtent.width;
//         framebufferInfo.height = vkc.swapChainExtent.height;
//         framebufferInfo.layers = 1;
//         if (vkCreateFramebuffer(vkc.device, &framebufferInfo, NULL, &vkc.offscreenFramebuffer) != VK_SUCCESS)
//         {
//             SDL_Log("Failed to create offscreen framebuffer!\n");
//             return;
//         }
//     }
// }

// void initVulkanDevice(void)
// {
//     createInstance();
//     findPhysicalDevice();
//     createWindowSurface();
//     queryQueueFamilies();
//     createLogicalDevice();
//     getQueues();
//     getSurfaceFormats();
//     createSwapchain();
//     createSwapchainImageViews();
//     createCommandPool();
//     createDescriptorPool();
//     createDescriptorSetLayouts();
//     createTextureSampler();
//     createUniformBuffers();
// }

// void initVulkan(void)
// {
//     createDepthResources();
//     createOffscreenTexture();
//     allocateCommandBuffers();
//     createSyncObjects();
//     createRenderPass();
//     createPipelineCache();
//     createGraphicsPipelines();
//     createFramebuffers();
//     createDescriptorSets();
// }

// void cleanupVulkan(void)
// {
//     vkDestroyImageView(vkc.device, vkc.depthImageView, NULL);
//     vkDestroyImage(vkc.device, vkc.depthImage, NULL);
//     vkFreeMemory(vkc.device, vkc.depthImageMemory, NULL);
//     vkDestroySampler(vkc.device, vkc.textureSampler, NULL);
//     vkDestroyImageView(vkc.device, vkc.offscreenTexture.imageView, NULL);
//     vkDestroyImage(vkc.device, vkc.offscreenTexture.image, NULL);
//     vkFreeMemory(vkc.device, vkc.offscreenTexture.deviceMemory, NULL);
//     for (int i = 0; i < vkc.swapChainImageCount; i++) {
//         vkDestroyFramebuffer(vkc.device, vkc.swapChainFramebuffers[i], NULL);
//     }
//     free(vkc.swapChainFramebuffers);
//     vkDestroyFramebuffer(vkc.device, vkc.offscreenFramebuffer, NULL);
//     vkDestroyBuffer(vkc.device, vkc.uniformBuffers.vsShared.buffer, NULL);
//     vkFreeMemory(vkc.device, vkc.uniformBuffers.vsShared.deviceMemory, NULL);
//     vkDestroyBuffer(vkc.device, vkc.uniformBuffers.vsQuad.buffer, NULL);
//     vkFreeMemory(vkc.device, vkc.uniformBuffers.vsQuad.deviceMemory, NULL);
//     vkDestroyDescriptorPool(vkc.device, vkc.descriptorPool, NULL);
//     vkDestroyDescriptorSetLayout(vkc.device, vkc.descriptorSetLayouts.textured, NULL);
//     vkDestroyPipeline(vkc.device, vkc.pipelines.quad, NULL);
//     vkDestroyPipeline(vkc.device, vkc.pipelines.shadedOffscreen, NULL);
//     vkDestroyPipelineCache(vkc.device, vkc.pipelineCache, NULL);
//     vkDestroyRenderPass(vkc.device, vkc.renderPass, NULL);
//     vkDestroyRenderPass(vkc.device, vkc.offscreenRenderPass, NULL);
//     vkDestroyPipelineLayout(vkc.device, vkc.pipelineLayouts.quad, NULL);
//     vkDestroyPipelineLayout(vkc.device, vkc.pipelineLayouts.shadedOffscreen, NULL);
//     for (int i = 0; i < vkc.swapChainImageCount; i++) {
//         vkDestroySemaphore(vkc.device, vkc.imageAvailableSemaphores[i], NULL);
//         vkDestroySemaphore(vkc.device, vkc.renderFinishedSemaphores[i], NULL);
//         vkDestroyFence(vkc.device, vkc.waitFences[i], NULL);
//     }
//     free(vkc.imageAvailableSemaphores);
//     free(vkc.renderFinishedSemaphores);
//     free(vkc.waitFences);
//     vkDestroyCommandPool(vkc.device, vkc.commandPool, NULL);
//     free(vkc.commandBuffers);
//     for (int i = 0; i < vkc.swapChainImageCount; i++) {
//         vkDestroyImageView(vkc.device, vkc.swapChainImageViews[i], NULL);
//     }
//     free(vkc.swapChainImages);
//     free(vkc.swapChainImageViews);
//     vkDestroySwapchainKHR(vkc.device, vkc.swapChain, NULL);
//     vkDestroySurfaceKHR(vkc.instance, vkc.surface, NULL);
//     vkDestroyDevice(vkc.device, NULL);
//     vkDestroyInstance(vkc.instance, NULL);
// }


#include "vkc.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

extern SDL_Window* window;

typedef struct {
    void* data;
    int len;
} FileData;

FileData readFile(const char* path)
{
    FILE* f = fopen(path, "rb");
    if (!f) {
        SDL_Log("Could not open %s!\n", path);
        FileData fd = { 0 };
        return fd;
    }
    fseek(f, 0, SEEK_END);
    FileData fd;
    fd.len = (int)ftell(f);
    fd.data = malloc(fd.len);
    fseek(f, 0, SEEK_SET);
    fread(fd.data, 1, fd.len, f);
    return fd;
}

void createInstance()
{
    VkApplicationInfo appInfo = { 0 };
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    unsigned int extensionCount;
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, NULL);
    SDL_Log("SDL required Vulkan extensions: %d\n", extensionCount);

    const char** ppExtensionNames = calloc(extensionCount, sizeof(const char*));
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, ppExtensionNames);
    for (unsigned i = 0; i < extensionCount; i++) {
        SDL_Log("Extension %d: %s\n", i, ppExtensionNames[i]);
    }

    VkInstanceCreateInfo createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = 0;
    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = ppExtensionNames;

    VkResult result = vkCreateInstance(&createInfo, NULL, &vkc.instance);
    if (result != VK_SUCCESS) {
        SDL_Log("Error initializing Vulkan, result=%d\n", result);
        return;
    }

    free(ppExtensionNames);
}

void findPhysicalDevice()
{
    vkc.physicalDevice = VK_NULL_HANDLE; // default value
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkc.instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        SDL_Log("Failed to find GPUs with Vulkan support!\n");
        return;
    }
    SDL_Log("Found %d GPUs with Vulkan support.\n", deviceCount);

    VkPhysicalDevice* devices = calloc(deviceCount, sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(vkc.instance, &deviceCount, devices);
    for (unsigned i = 0; i < deviceCount; i++)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(devices[i], &properties);
        SDL_Log("Device %d: %s\n", i, properties.deviceName);
    }

    vkc.physicalDevice = devices[0];

    free(devices);
}

void createWindowSurface()
{
    SDL_bool result = SDL_Vulkan_CreateSurface(window, vkc.instance, &vkc.surface);
    if (result == false) {
        SDL_Log("Error creating Vulkan surface: %s\n", SDL_GetError());
        return;
    }
}

void queryQueueFamilies()
{
    vkc.graphicsQueueFamilyIndex = UINT32_MAX;
    vkc.presentQueueFamilyIndex = UINT32_MAX;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vkc.physicalDevice, &queueFamilyCount, NULL);
    VkQueueFamilyProperties* queueFamilies = NULL;
    queueFamilies = malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(vkc.physicalDevice, &queueFamilyCount, queueFamilies);
    for (unsigned i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            vkc.graphicsQueueFamilyIndex = i;
        }
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(vkc.physicalDevice, i, vkc.surface, &presentSupport);
        if (presentSupport == VK_TRUE) {
            vkc.presentQueueFamilyIndex = i;
            if (vkc.graphicsQueueFamilyIndex == i) break; // queue that can do both
        }
    }
    if (vkc.graphicsQueueFamilyIndex == UINT32_MAX) {
        SDL_Log("Could not find queue index with VK_QUEUE_GRAPHICS_BIT!\n");
        return;
    }

    if (vkc.presentQueueFamilyIndex == UINT32_MAX) {
        SDL_Log("Could not find queue index with surface present support!\n");
        return;
    }
    free(queueFamilies);
}

void createLogicalDevice()
{
    VkDeviceQueueCreateInfo queueCreateInfo = { 0 };
    float queuePriority = 1.0f;
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = vkc.graphicsQueueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = { 0 };
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;
    const char* extensionNames[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    createInfo.ppEnabledExtensionNames = extensionNames;
    createInfo.enabledExtensionCount = 1;
    createInfo.enabledLayerCount = 0;

    VkResult result = vkCreateDevice(vkc.physicalDevice, &createInfo, NULL, &vkc.device);
    if (result != VK_SUCCESS) {
        SDL_Log("Error creating logical Vulkan device: %d\n", result);
    }
}

void getQueues()
{
    vkGetDeviceQueue(vkc.device, vkc.graphicsQueueFamilyIndex, 0, &vkc.graphicsQueue);
    vkGetDeviceQueue(vkc.device, vkc.presentQueueFamilyIndex, 0, &vkc.presentQueue);
}

void getSurfaceFormats()
{
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkc.physicalDevice, vkc.surface, &formatCount, NULL);

    VkSurfaceFormatKHR* formats = NULL;
    if (formatCount != 0)
    {
        formats = calloc(formatCount, sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(vkc.physicalDevice, vkc.surface, &formatCount, formats);
        vkc.surfaceFormat = formats[0]; // default format
        for (unsigned i = 0; i < formatCount; i++)
        {
            if (formats[i].format == VK_FORMAT_R8G8B8A8_SRGB &&
                formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
            {
                SDL_Log("Surface VK_FORMAT_R8G8B8A8_SRGB VK_COLORSPACE_SRGB_NONLINEAR_KHR.\n");
                vkc.surfaceFormat = formats[i];
                break;
            }

            if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
                formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
            {
                SDL_Log("Surface format VK_FORMAT_B8G8R8A8_SRGB VK_COLORSPACE_SRGB_NONLINEAR_KHR.\n");
                vkc.surfaceFormat = formats[i];
                break;
            }
        }
        free(formats);
    }
    else
    {
        free(formats);
        SDL_Log("No valid surface formats found!\n");
        return;
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkc.physicalDevice, vkc.surface, &presentModeCount, NULL);
    VkPresentModeKHR* presentModes = NULL;
    if (presentModeCount != 0)
    {
        presentModes = calloc(presentModeCount, sizeof(VkPresentModeKHR));
        vkGetPhysicalDeviceSurfacePresentModesKHR(vkc.physicalDevice, vkc.surface, &presentModeCount, presentModes);
        vkc.presentMode = presentModes[0]; // default present mode
        for (unsigned i = 0; i < presentModeCount; i++)
        {
            if (presentModes[i] == VK_PRESENT_MODE_FIFO_KHR) {
                SDL_Log("Present mode FIFO.\n");
                vkc.presentMode = presentModes[i];
                break;
            }
        }
    }
    else
    {
        SDL_Log("No valid present modes found!\n");
        return;
    }

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkc.physicalDevice, vkc.surface, &vkc.surfaceCapabilities);
    if (vkc.surfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        vkc.swapChainExtent = vkc.surfaceCapabilities.currentExtent;
    }
    else
    {
        int w, h;
        SDL_GetWindowSizeInPixels(window, &w, &h);
        VkExtent2D minExtent = vkc.surfaceCapabilities.minImageExtent;
        VkExtent2D maxExtent = vkc.surfaceCapabilities.maxImageExtent;
        vkc.swapChainExtent.width = SDL_clamp((uint32_t)w, minExtent.width, maxExtent.width);
        vkc.swapChainExtent.height = SDL_clamp((uint32_t)h, minExtent.height, maxExtent.height);
    }
    SDL_Log("Swap extent: %dx%d", vkc.swapChainExtent.width, vkc.swapChainExtent.height);
}

void createSwapchain()
{
    uint32_t imageCount = vkc.surfaceCapabilities.minImageCount + 1;
    if (imageCount > vkc.surfaceCapabilities.maxImageCount && vkc.surfaceCapabilities.maxImageCount > 0) {
        imageCount = vkc.surfaceCapabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vkc.surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = vkc.surfaceFormat.format;
    createInfo.imageColorSpace = vkc.surfaceFormat.colorSpace;
    createInfo.imageExtent = vkc.swapChainExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = vkc.surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = vkc.presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(vkc.device, &createInfo, NULL, &vkc.swapChain);
    if (result != VK_SUCCESS) {
        SDL_Log("Could not create swapchain: %d\n", result);
        return;
    }
}

VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo = { 0 };
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(vkc.device, &viewInfo, NULL, &imageView) != VK_SUCCESS) {
        SDL_Log("failed to create image view!\n");
        return (VkImageView) 0;
    }

    return imageView;
}

void createSwapchainImageViews()
{
    vkGetSwapchainImagesKHR(vkc.device, vkc.swapChain, &vkc.swapChainImageCount, NULL);
    vkc.swapChainImages = calloc(vkc.swapChainImageCount, sizeof(VkImage));
    vkGetSwapchainImagesKHR(vkc.device, vkc.swapChain, &vkc.swapChainImageCount, vkc.swapChainImages);
    SDL_Log("Swapchain has %d images.\n", vkc.swapChainImageCount);

    vkc.swapChainImageViews = calloc(vkc.swapChainImageCount, sizeof(VkImageView));

    for (int i = 0; i < vkc.swapChainImageCount; i++)
    {
        vkc.swapChainImageViews[i] = createImageView(vkc.swapChainImages[i], vkc.surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void createCommandPool()
{
    VkCommandPoolCreateInfo poolInfo = { 0 };
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = vkc.graphicsQueueFamilyIndex;
    VkResult result = vkCreateCommandPool(vkc.device, &poolInfo, NULL, &vkc.commandPool);
    if (result != VK_SUCCESS) {
        SDL_Log("Could not create command pool: %d\n", result);
        return;
    }
}

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vkc.physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    SDL_Log("failed to find suitable memory type!");
    return UINT32_MAX;
}

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory) {
    VkBufferCreateInfo bufferInfo = { 0 };
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vkc.device, &bufferInfo, NULL, buffer) != VK_SUCCESS) {
        SDL_Log("failed to create buffer!");
        return;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vkc.device, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(vkc.device, &allocInfo, NULL, bufferMemory) != VK_SUCCESS) {
        SDL_Log("failed to allocate buffer memory!");
        return;
    }

    vkBindBufferMemory(vkc.device, *buffer, *bufferMemory, 0);
}

void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
    VkMemoryPropertyFlags memoryProperties, VkImage *image, VkDeviceMemory *imageMemory)
{
    VkImageCreateInfo imageInfo = { 0 };
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0; // Optional

    if (vkCreateImage(vkc.device, &imageInfo, NULL, image) != VK_SUCCESS) {
        SDL_Log("failed to create image!\n");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vkc.device, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, memoryProperties);

    if (vkAllocateMemory(vkc.device, &allocInfo, NULL, imageMemory) != VK_SUCCESS) {
        SDL_Log("failed to allocate image memory!\n");
    }

    vkBindImageMemory(vkc.device, *image, *imageMemory, 0);
}

VkCommandBuffer beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vkc.commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(vkc.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = { 0 };
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = { 0 };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(vkc.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vkc.graphicsQueue);

    vkFreeCommandBuffers(vkc.device, vkc.commandPool, 1, &commandBuffer);
}

void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier = { 0 };
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        SDL_Log("unsupported layout transition!\n");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, NULL,
        0, NULL,
        1, &barrier
    );

    endSingleTimeCommands(commandBuffer);
}

void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region = { 0 };
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset.x = 0;
    region.imageOffset.y = 0;
    region.imageOffset.z = 0;

    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    endSingleTimeCommands(commandBuffer);
}

bool hasStencilComponent(VkFormat format) 
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat findSupportedFormat(const VkFormat* candidates, size_t candidatesCount, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (int i = 0; i < candidatesCount; i++)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(vkc.physicalDevice, candidates[i], &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return candidates[i];
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return candidates[i];
        }
    }
    SDL_Log("failed to find supported format!\n");
    return (VkFormat) 0;
}

VkFormat findDepthFormat()
{
    VkFormat candidates[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
    size_t candidatesCount = sizeof(candidates) / sizeof(candidates[0]);
    return findSupportedFormat(candidates, candidatesCount, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void createDepthResources()
{
    VkFormat depthFormat = findDepthFormat();
    createImage(vkc.swapChainExtent.width, vkc.swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vkc.depthImage, &vkc.depthImageMemory);
    vkc.depthImageView = createImageView(vkc.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void createOffscreenTexture()
{
    createImage(vkc.swapChainExtent.width, vkc.swapChainExtent.height, vkc.surfaceFormat.format, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &vkc.offscreenTexture.image, &vkc.offscreenTexture.deviceMemory);
    vkc.offscreenTexture.imageView = createImageView(vkc.offscreenTexture.image, vkc.surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
}

void createTextureSampler()
{
    VkSamplerCreateInfo samplerInfo = { 0 };
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties = {0};
    vkGetPhysicalDeviceProperties(vkc.physicalDevice, &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(vkc.device, &samplerInfo, NULL, &vkc.textureSampler) != VK_SUCCESS) {
        SDL_Log("failed to create texture sampler!\n");
    }
}

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion = {0};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vkc.uniformBuffers.vsShared.buffer, &vkc.uniformBuffers.vsShared.deviceMemory);
    vkMapMemory(vkc.device, vkc.uniformBuffers.vsShared.deviceMemory, 0, bufferSize, 0, &vkc.uniformBuffers.vsShared.mappedMemory);

    bufferSize = sizeof(PostProcUBO);
    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vkc.uniformBuffers.vsQuad.buffer, &vkc.uniformBuffers.vsQuad.deviceMemory);
    vkMapMemory(vkc.device, vkc.uniformBuffers.vsQuad.deviceMemory, 0, bufferSize, 0, &vkc.uniformBuffers.vsQuad.mappedMemory);
}

void allocateCommandBuffers()
{
    VkCommandBufferAllocateInfo allocInfo = { 0 };
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vkc.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = vkc.swapChainImageCount;

    vkc.commandBuffers = calloc(allocInfo.commandBufferCount, sizeof(VkCommandBuffer));

    VkResult result = vkAllocateCommandBuffers(vkc.device, &allocInfo, vkc.commandBuffers);
    if (result != VK_SUCCESS) {
        SDL_Log("Failed to allocate command buffer: %d\n", result);
        return;
    }
}

void createSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo = { 0 };
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = { 0 };
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // start in signaled state

    vkc.imageAvailableSemaphores = calloc(vkc.swapChainImageCount, sizeof(VkSemaphore));
    vkc.renderFinishedSemaphores = calloc(vkc.swapChainImageCount, sizeof(VkSemaphore));
    vkc.waitFences = calloc(vkc.swapChainImageCount, sizeof(VkFence));
    for (int i = 0; i < vkc.swapChainImageCount; i++) {
        if (vkCreateSemaphore(vkc.device, &semaphoreInfo, NULL, &vkc.imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vkc.device, &semaphoreInfo, NULL, &vkc.renderFinishedSemaphores[i]) != VK_SUCCESS) {
            SDL_Log("Failed to create semaphore!\n");
        }
        if (vkCreateFence(vkc.device, &fenceInfo, NULL, &vkc.waitFences[i]) != VK_SUCCESS)
        {
            SDL_Log("Failed to create fence!\n");
        }
    }
}

void createDescriptorSetLayouts()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = { 0 };
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBinding.pImmutableSamplers = NULL; // Optional

    VkDescriptorSetLayoutBinding sampler1LayoutBinding = { 0 };
    sampler1LayoutBinding.binding = 1;
    sampler1LayoutBinding.descriptorCount = 1;
    sampler1LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler1LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    sampler1LayoutBinding.pImmutableSamplers = NULL; // Optional

    VkDescriptorSetLayoutBinding sampler2LayoutBinding = { 0 };
    sampler2LayoutBinding.binding = 2;
    sampler2LayoutBinding.descriptorCount = 1;
    sampler2LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler2LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    sampler2LayoutBinding.pImmutableSamplers = NULL; // Optional

    VkDescriptorSetLayoutBinding sampler3LayoutBinding = { 0 };
    sampler3LayoutBinding.binding = 3;
    sampler3LayoutBinding.descriptorCount = 1;
    sampler3LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler3LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    sampler3LayoutBinding.pImmutableSamplers = NULL; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo = { 0 };
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    VkDescriptorSetLayoutBinding bindings[] = { uboLayoutBinding, sampler1LayoutBinding, sampler2LayoutBinding, sampler3LayoutBinding };
    layoutInfo.bindingCount = sizeof(bindings) / sizeof(bindings[0]);
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(vkc.device, &layoutInfo, NULL, &vkc.descriptorSetLayouts.textured) != VK_SUCCESS) {
        SDL_Log("failed to create descriptor set layout!");
        return;
    }
}


void createDescriptorPool()
{
    size_t poolSizesCount = 2;
    VkDescriptorPoolSize *poolSizes = calloc(poolSizesCount, sizeof(VkDescriptorPoolSize));
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 128;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 512;

    VkDescriptorPoolCreateInfo poolInfo = { 0 };
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = (uint32_t) poolSizesCount;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = 128;

    if (vkCreateDescriptorPool(vkc.device, &poolInfo, NULL, &vkc.descriptorPool) != VK_SUCCESS) {
        SDL_Log("failed to create descriptor pool!");
        free(poolSizes);
        return;
    }
    free(poolSizes);
}

void createDescriptorSets()
{
    VkDescriptorSetAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = vkc.descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &vkc.descriptorSetLayouts.textured;

    if (vkAllocateDescriptorSets(vkc.device, &allocInfo, &vkc.descriptorSets.quad) != VK_SUCCESS) {
        SDL_Log("failed to allocate descriptor set (quad)!");
        return;
    }

    {
        VkDescriptorBufferInfo bufferInfo = { 0 };
        bufferInfo.buffer = vkc.uniformBuffers.vsQuad.buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(PostProcUBO);

        VkDescriptorImageInfo imageInfo = { 0 };
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = vkc.offscreenTexture.imageView;
        imageInfo.sampler = vkc.textureSampler;

        size_t descriptorWritesCount = 2;
        VkWriteDescriptorSet *descriptorWrites = calloc(descriptorWritesCount, sizeof(VkWriteDescriptorSet));

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = vkc.descriptorSets.quad;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = vkc.descriptorSets.quad;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(vkc.device, (uint32_t) descriptorWritesCount, descriptorWrites, 0, NULL);
        free(descriptorWrites);
    }
}

VkShaderModule createShaderModule(FileData fd)
{
    VkShaderModuleCreateInfo createInfo = { 0 };
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = fd.len;
    createInfo.pCode = fd.data;
    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(vkc.device, &createInfo, NULL, &shaderModule);
    if (result != VK_SUCCESS) {
        SDL_Log("vkCreateShaderModule: %d\n", result);
    }
    return shaderModule;
}

void createPipelineCache()
{
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = { 0 };
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    if (vkCreatePipelineCache(vkc.device, &pipelineCacheCreateInfo, NULL, &vkc.pipelineCache) != VK_SUCCESS) {
        SDL_Log("failed to create pipeline cache\n");
        return;
    }
}

void createGraphicsPipelines()
{
    FileData vertShaderCodeShaded = readFile("shader.vert.spv");
    FileData fragShaderCodeShaded = readFile("shader.frag.spv");
    VkShaderModule vertShaderModuleShaded = createShaderModule(vertShaderCodeShaded);
    VkShaderModule fragShaderModuleShaded = createShaderModule(fragShaderCodeShaded);

    FileData vertShaderCodeQuad = readFile("quad.vert.spv");
    FileData fragShaderCodeQuad = readFile("quad.frag.spv");
    VkShaderModule vertShaderModuleQuad = createShaderModule(vertShaderCodeQuad);
    VkShaderModule fragShaderModuleQuad = createShaderModule(fragShaderCodeQuad);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = { 0 };
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModuleShaded;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = { 0 };
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModuleShaded;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState = { 0 };
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkVertexInputBindingDescription bindingDescription = { 0 };
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescriptions[5] = { 0 };
    const int attributeDescriptionsCount = sizeof(attributeDescriptions) / sizeof(attributeDescriptions[0]);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, normal);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

    attributeDescriptions[4].binding = 0;
    attributeDescriptions[4].location = 4;
    attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[4].offset = offsetof(Vertex, tangent);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = { 0 };
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptionsCount;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = { 0 };
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = { 0 };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)vkc.swapChainExtent.width;
    viewport.height = (float)vkc.swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = { 0 };
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = vkc.swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState = { 0 };
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = { 0 };
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling = { 0 };
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = NULL; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment = { 0 };
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending = { 0 };
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineDepthStencilStateCreateInfo depthStencil = { 0 };
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    VkStencilOpState emptyStencilOpState = { 0 };
    depthStencil.front = emptyStencilOpState; // Optional
    depthStencil.back = emptyStencilOpState; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = { 0 };
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &vkc.descriptorSetLayouts.textured;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = NULL; // Optional

    VkResult result = vkCreatePipelineLayout(vkc.device, &pipelineLayoutInfo, NULL, &vkc.pipelineLayouts.shadedOffscreen);
    if (result != VK_SUCCESS) {
        SDL_Log("failed to create pipeline layout (offscreen)!");
    }
    result = vkCreatePipelineLayout(vkc.device, &pipelineLayoutInfo, NULL, &vkc.pipelineLayouts.quad);
    if (result != VK_SUCCESS) {
        SDL_Log("failed to create pipeline layout (quad)!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = { 0 };
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = vkc.pipelineLayouts.quad;
    pipelineInfo.renderPass = vkc.offscreenRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    result = vkCreateGraphicsPipelines(vkc.device, vkc.pipelineCache, 1, &pipelineInfo, NULL, &vkc.pipelines.shadedOffscreen);
    if (result != VK_SUCCESS) {
        SDL_Log("failed to create graphics pipeline (offscreen pass)!");
    }

    shaderStages[0].module = vertShaderModuleQuad;
    shaderStages[1].module = fragShaderModuleQuad;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = NULL;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = NULL;
    pipelineInfo.renderPass = vkc.renderPass;
    rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT; // triangle in quad.vert faces backwards

    result = vkCreateGraphicsPipelines(vkc.device, vkc.pipelineCache, 1, &pipelineInfo, NULL, &vkc.pipelines.quad);
    if (result != VK_SUCCESS) {
        SDL_Log("failed to create graphics pipeline (quad pass)!");
    }

    vkDestroyShaderModule(vkc.device, fragShaderModuleShaded, NULL);
    vkDestroyShaderModule(vkc.device, vertShaderModuleShaded, NULL);
    free(vertShaderCodeShaded.data);
    free(fragShaderCodeShaded.data);
    vkDestroyShaderModule(vkc.device, fragShaderModuleQuad, NULL);
    vkDestroyShaderModule(vkc.device, vertShaderModuleQuad, NULL);
    free(vertShaderCodeQuad.data);
    free(fragShaderCodeQuad.data);
}

void createRenderPass()
{
    VkAttachmentDescription colorAttachment = { 0 };
    colorAttachment.format = vkc.surfaceFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference colorAttachmentRef = { 0 };
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = { 0 };
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = { 0 };
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = { 0 };
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependencies[2] = {0};
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo = { 0 };
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };
    renderPassInfo.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 2;
    renderPassInfo.pDependencies = dependencies;

    VkResult result = vkCreateRenderPass(vkc.device, &renderPassInfo, NULL, &vkc.offscreenRenderPass);
    if (result != VK_SUCCESS) {
        SDL_Log("failed to create render pass (offscreen)!\n");
    }


    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    dependencies[0].dependencyFlags = 0;

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 0;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].srcAccessMask = 0;
    dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[1].dependencyFlags = 0;

    result = vkCreateRenderPass(vkc.device, &renderPassInfo, NULL, &vkc.renderPass);
    if (result != VK_SUCCESS) {
        SDL_Log("failed to create render pass (to screen)!\n");
    }
}

void createFramebuffers() {
    // one framebuffer for every swap chain image
    vkc.swapChainFramebuffers = calloc(vkc.swapChainImageCount, sizeof(VkFramebuffer));
    for (size_t i = 0; i < vkc.swapChainImageCount; i++) {
        VkImageView attachments[] = {
            vkc.swapChainImageViews[i],
            vkc.depthImageView,
        };

        VkFramebufferCreateInfo framebufferInfo = { 0 };
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = vkc.renderPass;
        framebufferInfo.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = vkc.swapChainExtent.width;
        framebufferInfo.height = vkc.swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(vkc.device, &framebufferInfo, NULL, &vkc.swapChainFramebuffers[i]) != VK_SUCCESS) {
            SDL_Log("failed to create framebuffer!");
            return;
        }
    }

    // framebuffer for offscreen render target
    {
        VkImageView attachments[] = {
            vkc.offscreenTexture.imageView,
            vkc.depthImageView,
        };

        VkFramebufferCreateInfo framebufferInfo = { 0 };
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = vkc.offscreenRenderPass;
        framebufferInfo.attachmentCount = sizeof(attachments) / sizeof(attachments[0]);
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = vkc.swapChainExtent.width;
        framebufferInfo.height = vkc.swapChainExtent.height;
        framebufferInfo.layers = 1;
        if (vkCreateFramebuffer(vkc.device, &framebufferInfo, NULL, &vkc.offscreenFramebuffer) != VK_SUCCESS)
        {
            SDL_Log("Failed to create offscreen framebuffer!\n");
            return;
        }
    }
}

void initVulkanDevice()
{
    createInstance();
    findPhysicalDevice();
    createWindowSurface();
    queryQueueFamilies();
    createLogicalDevice();
    getQueues();
    getSurfaceFormats();
    createSwapchain();
    createSwapchainImageViews();
    createCommandPool();
    createDescriptorPool();
    createDescriptorSetLayouts();
    createTextureSampler();
    createUniformBuffers();
}

void initVulkan()
{
    createDepthResources();
    createOffscreenTexture();
    allocateCommandBuffers();
    createSyncObjects();
    createRenderPass();
    createPipelineCache();
    createGraphicsPipelines();
    createFramebuffers();
    createDescriptorSets();
}

void cleanupVulkan()
{
    vkDestroyImageView(vkc.device, vkc.depthImageView, NULL);
    vkDestroyImage(vkc.device, vkc.depthImage, NULL);
    vkFreeMemory(vkc.device, vkc.depthImageMemory, NULL);
    vkDestroySampler(vkc.device, vkc.textureSampler, NULL);
    vkDestroyImageView(vkc.device, vkc.offscreenTexture.imageView, NULL);
    vkDestroyImage(vkc.device, vkc.offscreenTexture.image, NULL);
    vkFreeMemory(vkc.device, vkc.offscreenTexture.deviceMemory, NULL);
    for (int i = 0; i < vkc.swapChainImageCount; i++) {
        vkDestroyFramebuffer(vkc.device, vkc.swapChainFramebuffers[i], NULL);
    }
    free(vkc.swapChainFramebuffers);
    vkDestroyFramebuffer(vkc.device, vkc.offscreenFramebuffer, NULL);
    vkDestroyBuffer(vkc.device, vkc.uniformBuffers.vsShared.buffer, NULL);
    vkFreeMemory(vkc.device, vkc.uniformBuffers.vsShared.deviceMemory, NULL);
    vkDestroyBuffer(vkc.device, vkc.uniformBuffers.vsQuad.buffer, NULL);
    vkFreeMemory(vkc.device, vkc.uniformBuffers.vsQuad.deviceMemory, NULL);
    vkDestroyDescriptorPool(vkc.device, vkc.descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(vkc.device, vkc.descriptorSetLayouts.textured, NULL);
    vkDestroyPipeline(vkc.device, vkc.pipelines.quad, NULL);
    vkDestroyPipeline(vkc.device, vkc.pipelines.shadedOffscreen, NULL);
    vkDestroyPipelineCache(vkc.device, vkc.pipelineCache, NULL);
    vkDestroyRenderPass(vkc.device, vkc.renderPass, NULL);
    vkDestroyRenderPass(vkc.device, vkc.offscreenRenderPass, NULL);
    vkDestroyPipelineLayout(vkc.device, vkc.pipelineLayouts.quad, NULL);
    vkDestroyPipelineLayout(vkc.device, vkc.pipelineLayouts.shadedOffscreen, NULL);
    for (int i = 0; i < vkc.swapChainImageCount; i++) {
        vkDestroySemaphore(vkc.device, vkc.imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(vkc.device, vkc.renderFinishedSemaphores[i], NULL);
        vkDestroyFence(vkc.device, vkc.waitFences[i], NULL);
    }
    free(vkc.imageAvailableSemaphores);
    free(vkc.renderFinishedSemaphores);
    free(vkc.waitFences);
    vkDestroyCommandPool(vkc.device, vkc.commandPool, NULL);
    free(vkc.commandBuffers);
    for (int i = 0; i < vkc.swapChainImageCount; i++) {
        vkDestroyImageView(vkc.device, vkc.swapChainImageViews[i], NULL);
    }
    free(vkc.swapChainImages);
    free(vkc.swapChainImageViews);
    vkDestroySwapchainKHR(vkc.device, vkc.swapChain, NULL);
    vkDestroySurfaceKHR(vkc.instance, vkc.surface, NULL);
    vkDestroyDevice(vkc.device, NULL);
    vkDestroyInstance(vkc.instance, NULL);
}
