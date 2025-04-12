#include "frame_data.h"
#include "renderer_p\renderer.h"
#include "world_p\components.h"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"


inline static glm::mat4 getUIMatrix() {
    vk::Extent2D extent = rfct::renderer::getRen().getWindow().getExtent();
    float width = static_cast<float>(extent.width);
    float height = static_cast<float>(extent.height);

    return glm::ortho(0.0f, width, 0.0f, height);
}

rfct::frameData::frameData(vk::Device device, VmaAllocator& allocator, vk::Fence lastFramePresentFinishedFence, vk::Fence thisFramePresentFinishedFence)
    : m_device(device), m_allocator(allocator), m_lastFrameRenderFinishedFence(lastFramePresentFinishedFence), m_thisFrameRenderFinishedFence(thisFramePresentFinishedFence), m_descriptors(RFCT_FRAMES_IN_FLIGHT) {

    vk::CommandPoolCreateInfo poolInfo{
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        renderer::getRen().getDeviceWrapper().getQueueManager().getGraphicsQueueFamilyIndex()
    };
    m_sceneCommandPool = device.createCommandPoolUnique(poolInfo);
    m_debugDrawCommandPool = device.createCommandPoolUnique(poolInfo);
    m_uiCommandPool = device.createCommandPoolUnique(poolInfo);

    vk::CommandBufferAllocateInfo allocInfoScene{*m_sceneCommandPool, vk::CommandBufferLevel::ePrimary, 1};
    auto commandBuffersScene = device.allocateCommandBuffersUnique(allocInfoScene);
    m_sceneCommandBuffer = std::move(commandBuffersScene[0]);

    vk::CommandBufferAllocateInfo allocInfoDebugDraw{ *m_debugDrawCommandPool, vk::CommandBufferLevel::ePrimary, 1 };
    auto commandBuffersDebugDraw = device.allocateCommandBuffersUnique(allocInfoDebugDraw);
    m_debugDrawCommandBuffer = std::move(commandBuffersDebugDraw[0]);

    vk::CommandBufferAllocateInfo allocInfoUI{ *m_uiCommandPool, vk::CommandBufferLevel::ePrimary, 1 };
    auto commandBuffersui = device.allocateCommandBuffersUnique(allocInfoUI);
    m_uiCommandBuffer = std::move(commandBuffersui[0]);

    vk::FenceCreateInfo fenceInfo{ vk::FenceCreateFlagBits::eSignaled };
    m_renderingFence = device.createFenceUnique(fenceInfo);

    vk::SemaphoreCreateInfo semaphoreInfo{};
    m_ImageAvaibleSemaphore = device.createSemaphoreUnique(semaphoreInfo);

    m_sceneFinishedSemaphore = device.createSemaphoreUnique(semaphoreInfo);
    m_debugDrawFinishedSemaphore = device.createSemaphoreUnique(semaphoreInfo);
    m_renderFinishedSemaphore = device.createSemaphoreUnique(semaphoreInfo);

	for (size_t i = 0; i < RFCT_FRAMES_IN_FLIGHT; i++)
	{
        m_descriptors.bindCameraUbo(m_cameraUbo[i].getBuffer(), i);
	}
	m_UIcameradescriptors.bindCameraUbo(m_UIcameraUbo.getBuffer());

}

void rfct::frameData::prepareFrame(uint32_t BufferIndex)
{
    m_cameraUbo[BufferIndex].updateViewProj(getVPMatrix());
    m_UIcameraUbo.updateViewProj(getUIMatrix());
}

void rfct::frameData::waitForAllFences()
{
    RFCT_PROFILE_SCOPE("fences wait");
    RFCT_VULKAN_CHECK(m_device.waitForFences(1, &m_thisFrameRenderFinishedFence, VK_TRUE, UINT64_MAX));
}

void rfct::frameData::resetAllFences()
{
    RFCT_PROFILE_SCOPE("fences reset");
    RFCT_VULKAN_CHECK(m_device.resetFences(1, &m_thisFrameRenderFinishedFence));
}

vk::SubmitInfo rfct::frameData::sceneSubmitInfo() const
{
    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };


    return vk::SubmitInfo()
        .setWaitSemaphores(m_ImageAvaibleSemaphore.get())
        .setWaitDstStageMask(waitStages)
        .setCommandBuffers(m_sceneCommandBuffer.get())
        .setSignalSemaphores(m_sceneFinishedSemaphore.get());
}
vk::SubmitInfo rfct::frameData::debugDrawSubmitInfo() const
{
    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

    return vk::SubmitInfo()
        .setWaitSemaphores(m_sceneFinishedSemaphore.get())
        .setWaitDstStageMask(waitStages)
        .setCommandBuffers(m_debugDrawCommandBuffer.get())
        .setSignalSemaphores(m_debugDrawFinishedSemaphore.get());
}

vk::SubmitInfo rfct::frameData::uiSubmitInfo() const
{
    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

    return vk::SubmitInfo()
        .setWaitSemaphores(m_debugDrawFinishedSemaphore.get())
        .setWaitDstStageMask(waitStages)
        .setCommandBuffers(m_uiCommandBuffer.get())
        .setSignalSemaphores(m_renderFinishedSemaphore.get());
}

