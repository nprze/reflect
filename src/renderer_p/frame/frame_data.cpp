#include "frame_data.h"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include "renderer_p/renderer.h"
#include "world_p/components.h"
#include "world_p/camera/camera.h"
#include "world_p/world.h"
#include "renderer_p/components/renderer_components.h"


rfct::frameSyncDataTemp::frameSyncDataTemp(RfctVulkanMemAllocator& allocatorWrapper, RfctQueue& queue, vk::Device device, 
    vk::Fence lastFramePresentFinishedFence, vk::Fence thisFramePresentFinishedFence) : 
        m_lastFrameRenderFinishedFence(lastFramePresentFinishedFence), m_thisFrameRenderFinishedFence(thisFramePresentFinishedFence) {
    RFCT_PROFILE_FUNCTION();
    vk::CommandPoolCreateInfo poolInfo {
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        queue.GetGraphicsQueueFamilyIndex()
    };
    m_sceneCommandPool = device.createCommandPoolUnique(poolInfo).value;
    m_debugDrawCommandPool = device.createCommandPoolUnique(poolInfo).value;
    m_uiCommandPool = device.createCommandPoolUnique(poolInfo).value;

    vk::CommandBufferAllocateInfo allocInfoScene{*m_sceneCommandPool, vk::CommandBufferLevel::ePrimary, 1};
    auto commandBuffersScene = device.allocateCommandBuffersUnique(allocInfoScene);
    m_sceneCommandBuffer = std::move(commandBuffersScene.value[0]);

    vk::CommandBufferAllocateInfo allocInfoDebugDraw{ *m_debugDrawCommandPool, vk::CommandBufferLevel::ePrimary, 1 };
    auto commandBuffersDebugDraw = device.allocateCommandBuffersUnique(allocInfoDebugDraw);
    m_debugDrawCommandBuffer = std::move(commandBuffersDebugDraw.value[0]);

    vk::CommandBufferAllocateInfo allocInfoUI{ *m_uiCommandPool, vk::CommandBufferLevel::ePrimary, 1 };
    auto commandBuffersui = device.allocateCommandBuffersUnique(allocInfoUI);
    m_uiCommandBuffer = std::move(commandBuffersui.value[0]);


    vk::FenceCreateInfo fenceInfo{ vk::FenceCreateFlagBits::eSignaled };
    m_renderingFence = device.createFenceUnique(fenceInfo).value;

    vk::SemaphoreCreateInfo semaphoreInfo{};
    m_ImageAvaibleSemaphore = device.createSemaphoreUnique(semaphoreInfo).value;

    m_sceneFinishedSemaphore = device.createSemaphoreUnique(semaphoreInfo).value;
    m_debugDrawFinishedSemaphore = device.createSemaphoreUnique(semaphoreInfo).value;
    m_bloomFinishedSemaphore = device.createSemaphoreUnique(semaphoreInfo).value;
    m_renderFinishedSemaphore = device.createSemaphoreUnique(semaphoreInfo).value;
}

void rfct::frameSyncDataTemp::WaitForFences(vk::Device device) {
    RFCT_PROFILE_FUNCTION();
    RFCT_VULKAN_CHECK(device.waitForFences(1, &m_thisFrameRenderFinishedFence, VK_TRUE, UINT64_MAX));
}

void rfct::frameSyncDataTemp::ResetFences(vk::Device device) {
	RFCT_PROFILE_FUNCTION();
    RFCT_VULKAN_CHECK(device.resetFences(1, &m_thisFrameRenderFinishedFence));
}

vk::SubmitInfo rfct::frameSyncDataTemp::sceneSubmitInfo(const frameContext& ctx) const {
    return vk::SubmitInfo()
        .setWaitSemaphores(m_ImageAvaibleSemaphore.get())
        .setCommandBuffers(m_sceneCommandBuffer.get())
        .setSignalSemaphores(m_sceneFinishedSemaphore.get());
}
vk::SubmitInfo rfct::frameSyncDataTemp::bloomSubmitInfo(const frameContext& ctx) const {
    return vk::SubmitInfo()
        .setWaitSemaphores(m_sceneFinishedSemaphore.get())
        .setCommandBuffers(m_BloomCommandBuffer)
        .setSignalSemaphores(m_bloomFinishedSemaphore.get());
}
vk::SubmitInfo rfct::frameSyncDataTemp::debugDrawSubmitInfo(const frameContext& ctx) const {
    RFCT_ASSERT(ctx.renderDebugDraw);
    return vk::SubmitInfo()
        .setWaitSemaphores(m_bloomFinishedSemaphore.get())
        .setCommandBuffers(m_debugDrawCommandBuffer.get())
        .setSignalSemaphores(m_debugDrawFinishedSemaphore.get());
}

vk::SubmitInfo rfct::frameSyncDataTemp::uiSubmitInfo(const frameContext& ctx) const {
    return vk::SubmitInfo()
        .setWaitSemaphores((ctx.renderDebugDraw ? m_debugDrawFinishedSemaphore.get() : m_bloomFinishedSemaphore.get()))
        .setCommandBuffers(m_uiCommandBuffer.get())
        .setSignalSemaphores(m_renderFinishedSemaphore.get());
}

