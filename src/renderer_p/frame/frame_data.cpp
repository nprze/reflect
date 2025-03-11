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
	//vk::Fence fences[] = { m_sceneInRenderFence.get(), m_debugDrawTrianglesInRenderFence.get() };
    RFCT_VULKAN_CHECK(m_device.waitForFences(1, &m_sceneInRenderFence.get(), VK_TRUE, UINT64_MAX));
}

void rfct::frameData::resetAllFences()
{
    RFCT_VULKAN_CHECK(m_device.resetFences(1, &m_sceneInRenderFence.get()));
}

vk::SubmitInfo rfct::frameData::submitInfo()
{
    vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    return vk::SubmitInfo()
        .setWaitSemaphores(m_imageAvailableSemaphore.get())
        .setWaitDstStageMask(waitStages)
        .setSignalSemaphores(m_renderFinishedSemaphore.get());
}

