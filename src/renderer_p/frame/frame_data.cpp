#include "frame_data.h"
#include "renderer_p/renderer.h"
#include "world_p/components.h"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include "world_p/camera/camera.h"
#include "world_p/world.h"  

inline static glm::mat4 getUIMatrix()
{
    vk::Extent2D extent = rfct::renderer::getRen().getWindow().getExtent();
    float width = static_cast<float>(extent.width);
    float height = static_cast<float>(extent.height);

    glm::mat4 screenRot = glm::rotate(glm::mat4(1), glm::radians(rfct::world::getWorld().screenViewTransformDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
    return screenRot * glm::ortho(0.0f, width, 0.0f, height);
}

rfct::frameData::frameData(vk::Device device, VmaAllocator& allocator, vk::Fence lastFramePresentFinishedFence, vk::Fence thisFramePresentFinishedFence)
    : m_lastFrameRenderFinishedFence(lastFramePresentFinishedFence), m_thisFrameRenderFinishedFence(thisFramePresentFinishedFence), m_descriptors(RFCT_FRAMES_IN_FLIGHT)
{

    vk::CommandPoolCreateInfo poolInfo 
    {
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
    m_bloomFinishedSemaphore = renderer::getRen().getDevice().createSemaphoreUnique(semaphoreInfo);
    m_renderFinishedSemaphore = device.createSemaphoreUnique(semaphoreInfo);

	for (size_t i = 0; i < RFCT_FRAMES_IN_FLIGHT; i++)
	{
        m_descriptors.bindCameraUbo(m_cameraUbo[i].getBuffer(), i);
	}
	m_UIcameradescriptors.bindCameraUbo(m_UIcameraUbo.getBuffer(), 0);

}

void rfct::frameData::prepareFrame(const frameContext& ctx, uint32_t BufferIndex)
{
    m_cameraUbo[BufferIndex].updateUboData(getVPMatrix(), ctx.globalTime);
    m_UIcameraUbo.updateUboData(getUIMatrix(), ctx.globalTime);
}

void rfct::frameData::waitForFences()
{
    RFCT_PROFILE_SCOPE("fences wait");
    RFCT_VULKAN_CHECK(renderer::getRen().getDevice().waitForFences(1, &m_thisFrameRenderFinishedFence, VK_TRUE, UINT64_MAX));
}

void rfct::frameData::resetFences()
{
    RFCT_PROFILE_SCOPE("fences reset");
    RFCT_VULKAN_CHECK(renderer::getRen().getDevice().resetFences(1, &m_thisFrameRenderFinishedFence));
}

vk::SubmitInfo rfct::frameData::sceneSubmitInfo(const frameContext& ctx) const
{
    return vk::SubmitInfo()
        .setWaitSemaphores(m_ImageAvaibleSemaphore.get())
        .setCommandBuffers(m_sceneCommandBuffer.get())
        .setSignalSemaphores(m_sceneFinishedSemaphore.get());
}
vk::SubmitInfo rfct::frameData::bloomSubmitInfo(const frameContext& ctx) const
{
    return vk::SubmitInfo()
        .setWaitSemaphores(m_sceneFinishedSemaphore.get())
        .setCommandBuffers(m_BloomCommandBuffer)
        .setSignalSemaphores(m_bloomFinishedSemaphore.get());
}
vk::SubmitInfo rfct::frameData::debugDrawSubmitInfo(const frameContext& ctx) const
{
    RFCT_ASSERT(ctx.renderDebugDraw);
    return vk::SubmitInfo()
        .setWaitSemaphores(m_bloomFinishedSemaphore.get())
        .setCommandBuffers(m_debugDrawCommandBuffer.get())
        .setSignalSemaphores(m_debugDrawFinishedSemaphore.get());
}

vk::SubmitInfo rfct::frameData::uiSubmitInfo(const frameContext& ctx) const
{
    return vk::SubmitInfo()
        .setWaitSemaphores((ctx.renderDebugDraw ? m_debugDrawFinishedSemaphore.get() : m_bloomFinishedSemaphore.get()))
        .setCommandBuffers(m_uiCommandBuffer.get())
        .setSignalSemaphores(m_renderFinishedSemaphore.get());
}

