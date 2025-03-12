#include "frame_data.h"
#include "renderer_p\renderer.h"
rfct::frameData::frameData(vk::Device device, VmaAllocator& allocator)
    : m_device(device), m_allocator(allocator) {

    vk::CommandPoolCreateInfo poolInfo{
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        renderer::getRen().getDeviceWrapper().getQueueManager().getGraphicQueueFamilyIndex()
    };
    m_commandPool = device.createCommandPoolUnique(poolInfo);

    vk::CommandBufferAllocateInfo allocInfo{
        *m_commandPool, vk::CommandBufferLevel::ePrimary, 2
    };

    auto commandBuffers = device.allocateCommandBuffersUnique(allocInfo);

    m_sceneCommandBuffer = std::move(commandBuffers[0]);
    m_debugDrawTrianglesCommandBuffer = std::move(commandBuffers[1]);

    vk::FenceCreateInfo fenceInfo{ vk::FenceCreateFlagBits::eSignaled };
    m_sceneInRenderFence = device.createFenceUnique(fenceInfo);
    m_debugDrawTrianglesInRenderFence = device.createFenceUnique(fenceInfo);

    vk::SemaphoreCreateInfo semaphoreInfo{};
    m_imageAvailableSemaphore = device.createSemaphoreUnique(semaphoreInfo);
    m_renderFinishedSemaphore = device.createSemaphoreUnique(semaphoreInfo);
}

void rfct::frameData::waitForAllFences()
{
    RFCT_PROFILE_SCOPE("fences wait");
	vk::Fence fences[] = { m_sceneInRenderFence.get(), m_debugDrawTrianglesInRenderFence.get() };
    RFCT_VULKAN_CHECK(m_device.waitForFences(2, fences, VK_TRUE, UINT64_MAX));
}

void rfct::frameData::resetAllFences()
{
    RFCT_PROFILE_SCOPE("fences reset");
    vk::Fence fences[] = { m_sceneInRenderFence.get(), m_debugDrawTrianglesInRenderFence.get() };
    RFCT_VULKAN_CHECK(m_device.resetFences(2, fences));
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
        .setCommandBuffers(m_debugDrawTrianglesCommandBuffer.get())
        .setSignalSemaphores(m_renderFinishedSemaphore.get());
}

