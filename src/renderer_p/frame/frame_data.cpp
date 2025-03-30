#include "frame_data.h"
#include "renderer_p\renderer.h"
#include "world_p\components.h"
rfct::frameData::frameData(vk::Device device, VmaAllocator& allocator)
    : m_device(device), m_allocator(allocator) {

    vk::CommandPoolCreateInfo poolInfo{
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        renderer::getRen().getDeviceWrapper().getQueueManager().getGraphicsQueueFamilyIndex()
    };
    m_commandPool = device.createCommandPoolUnique(poolInfo);

    vk::CommandBufferAllocateInfo allocInfo{
        *m_commandPool, vk::CommandBufferLevel::ePrimary, 3
    };

    auto commandBuffers = device.allocateCommandBuffersUnique(allocInfo);

    m_sceneCommandBuffer = std::move(commandBuffers[0]);
    m_debugDrawCommandBuffer = std::move(commandBuffers[1]);
    m_uiCommandBuffer = std::move(commandBuffers[2]);

    vk::FenceCreateInfo fenceInfo{ vk::FenceCreateFlagBits::eSignaled };
    m_sceneInRenderFence = device.createFenceUnique(fenceInfo);
    m_debugDrawTrianglesInRenderFence = device.createFenceUnique(fenceInfo);
    m_uiInRenderFence = device.createFenceUnique(fenceInfo);

    vk::SemaphoreCreateInfo semaphoreInfo{};
    m_imageAvailableSemaphore = device.createSemaphoreUnique(semaphoreInfo);
    m_renderFinishedSemaphore = device.createSemaphoreUnique(semaphoreInfo);

	m_descriptors.bindCameraUbo(m_cameraUbo.getBuffer());

}

void rfct::frameData::prepareFrame()
{
    m_cameraUbo.updateViewProj(getVPMatrix());
}

void rfct::frameData::waitForAllFences()
{
    RFCT_PROFILE_SCOPE("fences wait");
    RFCT_VULKAN_CHECK(m_device.waitForFences(1, &m_sceneInRenderFence.get(), VK_TRUE, UINT64_MAX));
}

void rfct::frameData::resetAllFences()
{
    RFCT_PROFILE_SCOPE("fences reset");
    RFCT_VULKAN_CHECK(m_device.resetFences(1, &m_sceneInRenderFence.get()));
}

vk::SubmitInfo rfct::frameData::sceneSubmitInfo()
{
    vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    return vk::SubmitInfo()
        .setWaitSemaphores(m_imageAvailableSemaphore.get())
        .setWaitDstStageMask(waitStages)
        .setCommandBuffers(m_sceneCommandBuffer.get())
        .setSignalSemaphores(m_renderFinishedSemaphore.get());
}
vk::SubmitInfo rfct::frameData::debugDrawSubmitInfo()
{
    vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    return vk::SubmitInfo()
        .setWaitSemaphores(m_renderFinishedSemaphore.get())
        .setWaitDstStageMask(waitStages)
        .setCommandBuffers(m_debugDrawCommandBuffer.get())
        .setSignalSemaphores(m_renderFinishedSemaphore.get());
}

vk::SubmitInfo rfct::frameData::uiSubmitInfo()
{
    vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    return vk::SubmitInfo()
        .setWaitSemaphores(m_renderFinishedSemaphore.get())
        .setWaitDstStageMask(waitStages)
        .setCommandBuffers(m_uiCommandBuffer.get())
        .setSignalSemaphores(m_renderFinishedSemaphore.get());
}

