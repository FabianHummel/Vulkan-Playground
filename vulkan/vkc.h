#pragma once

#include "types.h"
#include <vulkan/vulkan.h>

typedef struct {
    VkImage image;
    VkDeviceMemory deviceMemory;
    VkImageView imageView;
    VkSampler sampler;
} TextureSampler;

typedef struct {
    VkBuffer buffer;
    VkDeviceMemory deviceMemory;
    void* mappedMemory;
} MappedBuffer;

typedef struct {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;
    uint32_t graphicsQueueFamilyIndex;
    uint32_t presentQueueFamilyIndex;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    VkExtent2D swapChainExtent;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkSwapchainKHR swapChain;
    VkImage* swapChainImages;
    VkImageView* swapChainImageViews;
    int swapChainImageCount;
    VkCommandPool commandPool;
    VkCommandBuffer* commandBuffers;
    VkSemaphore* imageAvailableSemaphores;
    VkSemaphore* renderFinishedSemaphores;
    VkFence* waitFences;
    VkRenderPass renderPass;
    VkRenderPass offscreenRenderPass;
    struct {
        VkDescriptorSetLayout textured;
    } descriptorSetLayouts;
    VkDescriptorPool descriptorPool;
    struct {
        VkDescriptorSet shadedOffscreen;
        VkDescriptorSet quad;
    } descriptorSets;
    VkPipelineCache pipelineCache;
    struct {
        VkPipelineLayout shadedOffscreen;
        VkPipelineLayout quad;
    } pipelineLayouts;
    struct {
        VkPipeline shadedOffscreen;
        VkPipeline quad;
    } pipelines;
    VkFramebuffer* swapChainFramebuffers;
    VkFramebuffer offscreenFramebuffer;
    struct {
        MappedBuffer vsShared;
        MappedBuffer vsQuad;
    } uniformBuffers;

    TextureSampler modelTexture;
    TextureSampler offscreenTexture;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
} VKC;

extern VKC vkc;
