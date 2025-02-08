#include "C:/Users/Natal/Documents/games/reflect/CMakeFiles/reflect.dir/Debug/cmake_pch.hxx"
#include "renderer.h"
rfct::renderer rfct::renderer::ren;
rfct::renderer::renderer()
	: m_window(968, 422, "reflect"), m_instance(), m_device(), m_rasterizerPipeline(), m_framesInFlight()
{
    m_device.getSwapChain().createFrameBuffers();
}

void rfct::renderer::run()
{
	m_window.show();
	while (m_window.pollEvents()) draw();
}

void rfct::renderer::draw()
{
	frameData& frame = m_framesInFlight.getNextFrame();
    m_device.getDevice().waitForFences(1, &frame.m_inRenderFence.get(), VK_TRUE, 0);
    m_device.getDevice().resetFences(1, &frame.m_inRenderFence.get());
	uint32_t imageIndex = m_device.getSwapChain().acquireNextImage(frame.getImageAvailableSemaphore(), VK_NULL_HANDLE);
    
	m_rasterizerPipeline.recordAndSubmitCommandBuffer(frame, m_device.getSwapChain().getFrameBuffer(imageIndex), imageIndex);

    m_device.getDevice().waitForFences(1, &frame.m_inRenderFence.get(), VK_TRUE, UINT64_MAX);


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
    m_device.getQueueManager().getPresentQueue().presentKHR(&presentInfo);
}
