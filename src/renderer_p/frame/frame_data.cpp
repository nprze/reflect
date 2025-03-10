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
        *m_commandPool, vk::CommandBufferLevel::ePrimary, 1
    };
    m_sceneCommandBuffer = std::move(device.allocateCommandBuffersUnique(allocInfo)[0]);

    vk::FenceCreateInfo fenceInfo{ vk::FenceCreateFlagBits::eSignaled };
    m_inRenderFence = device.createFenceUnique(fenceInfo);

    vk::SemaphoreCreateInfo semaphoreInfo{};
    m_imageAvailableSemaphore = device.createSemaphoreUnique(semaphoreInfo);
    m_renderFinishedSemaphore = device.createSemaphoreUnique(semaphoreInfo);
}

vk::SubmitInfo rfct::frameData::submitInfo()
{
    vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    return vk::SubmitInfo()
        .setWaitSemaphores(m_imageAvailableSemaphore.get())
        .setWaitDstStageMask(waitStages)
        .setCommandBuffers(m_sceneCommandBuffer.get())
        .setSignalSemaphores(m_renderFinishedSemaphore.get());
}

