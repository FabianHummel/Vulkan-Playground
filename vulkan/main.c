#include "vkc.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <math.h>
#include "model.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

VKC vkc;
Model model;
SDL_Window* window;
bool running = true;

uint32_t currentFrame;

void input(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
    }
}

void update(double dt)
{
    //SDL_Log("dt: %.2f\n", dt* 1000);
}


//const Vertex vertices[] = {
//    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
//    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
//
//    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
//    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
//};
//
//const uint32_t indices[] = {
//    0, 1, 2, 2, 3, 0,
//    4, 5, 6, 6, 7, 4,
//};
//const uint32_t indicesCount = sizeof(indices) / sizeof(indices[0]);

Uint64 startTime;
void updateUniformBuffers(void) {

    Uint64 currentTime = SDL_GetTicks64() - startTime;
    float time = currentTime / 1000.0f;
    //SDL_Log("Time: %.2f\n", time);

    // parameters for transform, lighting (offscreen render pass)
    UniformBufferObject ubo = { 0 };
    vec3 rotAxis = { 0.0f, 1.0f, 0.0f };
    glm_mat4_identity(ubo.model);
    glm_rotate(ubo.model, time * glm_rad(90.0f), rotAxis);
    vec3 eye = { 0.0f, 2.0f, 3.0f };
    vec3 center = { 0.0f, 1.0f, 0.0f };
    vec3 up = { 0.0f, 1.0f, 0.0f };
    glm_lookat(eye, center, up, ubo.view);
    glm_perspective(glm_rad(45.0f), vkc.swapChainExtent.width / (float)vkc.swapChainExtent.height, 0.1f, 1000.0f, ubo.proj);
    ubo.proj[1][1] *= -1; // because Vulkan Y-axis is top-bottom, vs OpenGL Y-axis which is bottom-top
    glm_vec4(eye, 1.0f, ubo.lightPos);
    memcpy(vkc.uniformBuffers.vsShared.mappedMemory, &ubo, sizeof(ubo));

    // parameters for postprocessing render pass
    PostProcUBO ppUbo = { 0 };
    ppUbo.fbSize[0] = vkc.swapChainExtent.width;
    ppUbo.fbSize[1] = vkc.swapChainExtent.height;
    ppUbo.time = time;
    memcpy(vkc.uniformBuffers.vsQuad.mappedMemory, &ppUbo, sizeof(ppUbo));
}

void recordOffscreenRenderPass(Model* model, VkCommandBuffer commandBuffer)
{
    VkRenderPassBeginInfo renderPassInfo = { 0 };
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vkc.offscreenRenderPass;
    renderPassInfo.framebuffer = vkc.offscreenFramebuffer;
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = vkc.swapChainExtent;
    VkClearColorValue clearColor = { {0.0f, 0.0f, 0.0f, 1.0f} };
    VkClearValue clearValues[2] = { 0 };
    clearValues[0].color = clearColor;
    VkClearDepthStencilValue depthStencil = { 1.0f, 0 };
    clearValues[1].depthStencil = depthStencil;
    renderPassInfo.clearValueCount = sizeof(clearValues) / sizeof(clearValues[0]);
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkc.pipelines.shadedOffscreen);

    VkViewport viewport = { 0 };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)vkc.swapChainExtent.width;
    viewport.height = (float)vkc.swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = { 0 };
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = vkc.swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = { model->vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, model->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    updateUniformBuffers();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkc.pipelineLayouts.shadedOffscreen, 0, 1, &vkc.descriptorSets.shadedOffscreen, 0, NULL);
    vkCmdDrawIndexed(commandBuffer, model->indicesSize / sizeof(model->indices[0]), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);
}

void recordQuadRenderPass(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkRenderPassBeginInfo renderPassInfo = { 0 };
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vkc.renderPass;
    renderPassInfo.framebuffer = vkc.swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent = vkc.swapChainExtent;
    VkClearColorValue clearColor = { {0.0f, 0.0f, 0.0f, 1.0f} };
    VkClearValue clearValues[2] = { 0 };
    clearValues[0].color = clearColor;
    VkClearDepthStencilValue depthStencil = { 1.0f, 0 };
    clearValues[1].depthStencil = depthStencil;
    renderPassInfo.clearValueCount = sizeof(clearValues) / sizeof(clearValues[0]);
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkc.pipelines.quad);

    VkViewport viewport = { 0 };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)vkc.swapChainExtent.width;
    viewport.height = (float)vkc.swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = { 0 };
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = vkc.swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkc.pipelineLayouts.quad, 0, 1, &vkc.descriptorSets.quad, 0, NULL);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);
}

void recordCommandBuffer(Model *model, VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = NULL; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        SDL_Log("failed to begin recording command buffer!");
        return;
    }

    recordOffscreenRenderPass(model, commandBuffer);
    recordQuadRenderPass(commandBuffer, imageIndex);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        SDL_Log("failed to record command buffer!");
        return;
    }
}

void render()
{
    uint32_t frameIndex;
    VkResult result;
    VkClearColorValue clearColor = { { 0 } };
    VkPipelineStageFlags waitDestStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkSubmitInfo submitInfo = { 0 };
    VkPresentInfoKHR presentInfo = { 0 };
    VkImageSubresourceRange clearRange = { 0 };

    result = vkWaitForFences(vkc.device, 1, &vkc.waitFences[currentFrame], VK_FALSE, UINT64_MAX);
    if (result != VK_SUCCESS) {
        SDL_Log("vkWaitForFences(): %d\n", result);
        return;
    }
    result = vkResetFences(vkc.device, 1, &vkc.waitFences[currentFrame]);
    if (result != VK_SUCCESS) {
        SDL_Log("vkResetFences(): %d\n", result);
        return;
    }

    result = vkAcquireNextImageKHR(vkc.device,
        vkc.swapChain,
        UINT64_MAX,
        vkc.imageAvailableSemaphores[currentFrame],
        VK_NULL_HANDLE,
        &frameIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        SDL_Log("acquire next image: VK_ERROR_OUT_OF_DATE\n");
        return;
    }
    if ((result != VK_SUBOPTIMAL_KHR) && (result != VK_SUCCESS)) {
        SDL_Log("vkAcquireNextImageKHR(): %d\n", result);
        return;
    }

    recordCommandBuffer(&model, vkc.commandBuffers[currentFrame], frameIndex);
    
    // SUBMIT COMMAND BUFFER TO QUEUE
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &vkc.imageAvailableSemaphores[currentFrame];
    submitInfo.pWaitDstStageMask = &waitDestStageMask;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vkc.commandBuffers[currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vkc.renderFinishedSemaphores[currentFrame];
    result = vkQueueSubmit(vkc.graphicsQueue, 1, &submitInfo, vkc.waitFences[currentFrame]);

    if (result != VK_SUCCESS) {
        SDL_Log("vkQueueSubmit(): %d\n", result);
        return;
    }
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &vkc.renderFinishedSemaphores[currentFrame],
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vkc.swapChain;
    presentInfo.pImageIndices = &frameIndex;
    result = vkQueuePresentKHR(vkc.presentQueue, &presentInfo);
    if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
        SDL_Log("vkQueuePresentKHR(): VK_ERROR_OUT_OF_DATE or VK_SUBOPTIMAL\n");
        return;
    }

    if (result != VK_SUCCESS) {
        SDL_Log("vkQueuePresentKHR(): %d\n", result);
        return;
    }
    currentFrame = (currentFrame + 1) % vkc.swapChainImageCount;
}

extern void initVulkanDevice(void);
extern void initVulkan(const char *texturePath);
extern void cleanupVulkan(void);
extern void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("duck", 100, 100, 800, 600, SDL_WINDOW_VULKAN);

    initVulkanDevice();
    loadModel(&model, "/Users/fabian/Documents/Schule/4AHIF/FRSEC/vulkan/vulkan/models", "Duck.gltf");
    initVulkan(model.textureImagePath);

    startTime = SDL_GetTicks64();
    Uint64 ticks_prev = SDL_GetTicks64();
    while (running)
    {
        Uint64 ticks = SDL_GetTicks64();
        input();
        update((ticks - ticks_prev) / 1000.0);
        render();
        ticks_prev = ticks;
    }

    vkDeviceWaitIdle(vkc.device);
    destroyModel(&model);
    cleanupVulkan();
    SDL_DestroyWindow(window);
    return 0;
}
