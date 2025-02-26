#include "renderer.h"
#include <stdint.h>
rfct::renderer rfct::renderer::ren;
rfct::renderer::renderer()
	: m_window(968, 422, "reflect"), m_instance(), m_device(), m_rasterizerPipeline(), m_framesInFlight(), m_rayTracer()
{
    m_device.getSwapChain().createFrameBuffers();
}

void rfct::renderer::showWindow()
{
	m_window.show();
}

void rfct::renderer::render()
{
    RFCT_PROFILE_FUNCTION();
	frameData& frame = m_framesInFlight.getNextFrame();
    {
        RFCT_PROFILE_SCOPE("fences wait and reset");
        RFCT_VULKAN_CHECK(m_device.getDevice().waitForFences(1, &frame.m_inRenderFence.get(), VK_TRUE, 0));
    }

    uint32_t imageIndex;
    {
        RFCT_PROFILE_SCOPE("get sawpchain image");
        imageIndex = m_device.getSwapChain().acquireNextImage(frame.getImageAvailableSemaphore(), VK_NULL_HANDLE);

        if (imageIndex == -1)
        {
            return;
        }
        RFCT_MARK("acquired frame");
    }

    m_device.getDevice().resetFences(1, &frame.m_inRenderFence.get());
	m_rasterizerPipeline.recordAndSubmitCommandBuffer(frame, m_device.getSwapChain().getFrameBuffer(imageIndex), imageIndex);

    RFCT_VULKAN_CHECK(m_device.getDevice().waitForFences(1, &frame.m_inRenderFence.get(), VK_TRUE, UINT64_MAX));


    vk::PresentInfoKHR presentInfo{};
    presentInfo.sType = vk::StructureType::ePresentInfoKHR;

    presentInfo.waitSemaphoreCount = 1;
    const vk::Semaphore& sem = frame.getRenderFinishedSemaphore();
    presentInfo.pWaitSemaphores = &sem;

    presentInfo.swapchainCount = 1;
    vk::SwapchainKHR sc = m_device.getSwapChain().getSwapChain();
    presentInfo.pSwapchains = &sc;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;
    RFCT_VULKAN_CHECK(m_device.getQueueManager().getPresentQueue().presentKHR(&presentInfo));
}

void rfct::renderer::setObjectName(void* objectHandle, const std::string& name, vk::ObjectType objectType)
{
#ifndef RFCT_VULKAN_DEBUG_OFF
    if (!m_instance.getDynamicLoader().vkSetDebugUtilsObjectNameEXT) {
        throw std::runtime_error("Failed to load vkSetDebugUtilsObjectNameEXT!");
    }

    vk::DebugUtilsObjectNameInfoEXT nameInfo{};
    nameInfo.objectType = objectType;
    nameInfo.objectHandle = (uintptr_t)(objectHandle);
    nameInfo.pObjectName = name.c_str();

    m_device.getDevice().setDebugUtilsObjectNameEXT(nameInfo, m_instance.getDynamicLoader());

#endif // RFCT_VULKAN_DEBUG_OFF

}
