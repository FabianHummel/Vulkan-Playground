#include "vkc.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <math.h>
#include "model.h"
#include "player.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

VKC vkc;
Player player;
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
        
        if (event.type == SDL_MOUSEMOTION) {
            player.yaw += event.motion.xrel / 5.0;
            player.pitch += event.motion.yrel / 5.0;
            player.pitch = SDL_clamp(player.pitch, -80.0, 80.0);
        }
    }
}

void update(double dt)
{
    SDL_PumpEvents();
    int numKeys;
    const Uint8 *keystate = SDL_GetKeyboardState(&numKeys);
    double dx = 0.0, dz = 0.0;
    for (int i = 0; i < numKeys; i++) {
        if (keystate[SDL_SCANCODE_A] ) {
            dx -= 1.0;
        }
        if (keystate[SDL_SCANCODE_D] ) {
            dx += 1.0;
        }
        if (keystate[SDL_SCANCODE_W] ) {
            dz -= 1.0;
        }
        if (keystate[SDL_SCANCODE_S] ) {
            dz += 1.0;
        }
    }
    
    double l = SDL_sqrt(dx*dx + dz*dz);
    if (dx != 0.0) {
        dx /= l;
    }
    if (dz != 0.0) {
        dz /= l;
    }
    
    vec3 up = { 0.0f, 1.0f, 0.0f };
    vec3 delta_pos = { -dx, 0, -dz };
    glm_vec3_rotate(delta_pos, -glm_rad(player.yaw), up);
    
    player.pos[0] += delta_pos[0] * dt * 300.0;
    player.pos[2] += delta_pos[2] * dt * 300.0;
}

Uint64 startTime;
void updateUniformBuffers(void)
{

    Uint64 currentTime = SDL_GetTicks64() - startTime;
    float time = currentTime / 1000.0f;
    //SDL_Log("Time: %.2f\n", time);

    // parameters for transform, lighting (offscreen render pass)
    UniformBufferObject ubo = { 0 };
    glm_mat4_identity(ubo.model);
    glm_mat4_identity(ubo.view);
    
    vec3 up = { 0.0f, 1.0f, 0.0f };
    glm_rotate(ubo.view, glm_rad(player.yaw), up);
    
    mat4 inverted;
    glm_mat4_inv(ubo.view, inverted);
    vec3 forward;
    glm_vec3(inverted[0], forward);
    glm_vec3_normalize(forward);
    
    glm_rotate(ubo.view, glm_rad(player.pitch), forward);
    
    glm_translate(ubo.view, player.pos);
    
    glm_perspective(glm_rad(45.0f), vkc.swapChainExtent.width / (float)vkc.swapChainExtent.height, 0.1f, 10000.0f, ubo.proj);
    ubo.proj[1][1] *= -1; // because Vulkan Y-axis is top-bottom, vs OpenGL Y-axis which is bottom-top
    vec3 light = { 0.0f, 100.0f, 0.0f};
    glm_vec4(light, 1.0f, ubo.lightPos);
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
    
    
    updateUniformBuffers();
    renderModel(model, commandBuffer);

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

void render(void)
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
    presentInfo.pWaitSemaphores = &vkc.renderFinishedSemaphores[currentFrame];
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
extern void initVulkan(void);
extern void cleanupVulkan(void);
extern void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("duck", 100, 100, 800, 600, SDL_WINDOW_VULKAN);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    initVulkanDevice();
    loadModel(&model, "/Users/fabian/Documents/Schule/4AHIF/FRSEC/vulkan/vulkan/models/sponza", "Sponza.gltf");
    initVulkan();
    
    player.pos[0] = 0.0;
    player.pos[1] = -100.0;
    player.pos[2] = 0.0;

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
