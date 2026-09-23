#include "renderer.h"

#include "job_system_p/job_system.h"
#include "frame/frame_data.h"
#include "world_p/scene.h"
#include "world_p/world.h"
#include "assets/assets_utils.h"

rfct::RfctRenderer* ren = nullptr;

rfct::RfctRenderer& rfct::GetRen() {
    return *ren;
}

bool SetRenderer(rfct::RfctRenderer* renderer) {
    ren = renderer;
    return true;
}

rfct::RfctRenderer::RfctRenderer(RFCT_RENDERER_ARGUMENTS)
	: m_uselessBool(SetRenderer(this)),
    m_window(RFCT_WINDOWS_WINDOW_ARGUMENTS RFCT_NATIVE_WINDOW_ANDROID_VAR),
    m_instance(), 
    m_surface(m_window.CreateSurface(m_instance.GetInstance())),
    m_device(m_instance.GetInstance(), m_surface.GetSurface()), 
    m_queue(m_device.GetDevice(), m_device.GetPhysicalDevice(), m_surface.GetSurface()),
    m_allocator(m_device.GetPhysicalDevice(), m_device.GetDevice(), m_instance.GetInstance()),
	m_swapChain(m_device.GetPhysicalDevice(), m_device.GetDevice(), m_surface.GetSurface()),
    m_framesInFlight(m_allocator, m_queue, m_device.GetDevice()), 
    m_bloomRes(m_queue, m_renderImages, m_renderImages.GetIntermediateRenderPass(), m_device.GetDevice()),
    m_debugDraw(m_renderImages.GetIntermediateRenderPass(), m_device.GetDevice()),
    m_UIPipeline(m_renderImages.GetUIRenderPass(), m_device.GetDevice())
{
    m_frameGraph.GetResources().CreateUniformBuffers(m_allocator, m_device.GetDevice());
    m_scenePass.CreatePassResources( m_pipelineManager, m_device.GetDevice());
}

void rfct::RfctRenderer::DestroyRenderer() {
    m_frameGraph.GetResources().DestroyResources();
    m_scenePass.DestroyPassResources(m_device.GetDevice());
    m_renderImages.CleanupResources(m_allocator, m_device.GetDevice());
    RfctUniformBuffer::DestroyUniformDescriptorSetLayout(m_device.GetDevice());
}

void rfct::RfctRenderer::UpdateWindow(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR) {
    RFCT_PROFILE_FUNCTION();
    // surface holder change on android
#ifdef ANDROID_BUILD
    m_device.getDevice().waitIdle();
    if (m_surface.surface) {
        m_instance.GetInstance().destroySurfaceKHR(m_surface.surface);
        m_surface.surface = VK_NULL_HANDLE;
    }
    m_window.destroyWind();
    m_window = AndroidWindow(RFCT_NATIVE_WINDOW_ANDROID_VAR);
    m_surface.newSurface(m_window.createSurface(GetInstance()));
    RFCT_WARN("destroying widnow and surface. creating new surface with width, height: ({}, {})", m_window.getExtent().width, m_window.getExtent().height);
#endif
};

void rfct::RfctRenderer::Render(RfctFrameContext& frameCtx) {
    RFCT_PROFILE_FUNCTION();
    RfctFrameGraphPerFrameResources& currentFrameFGResources = m_frameGraph.GetFrameResources(frameCtx.frameInFlightIndex);
	frameSyncDataTemp& frameSyncDataTemp = m_framesInFlight.GetNextFrame(frameCtx.frameInFlightIndex);
    {
        RFCT_PROFILE_SCOPE("fences wait");
        currentFrameFGResources.WaitImageRenderFinished(m_device.GetDevice());
    }

    uint32_t swapChainImageIndex;
    {
        RFCT_PROFILE_SCOPE("get sawpchain image");
        rfct::RfctSwapChain::RfctAcquireNextImageResult acquireImageResult = m_swapChain.AcquireNextImage(frameSyncDataTemp.m_ImageAvaibleSemaphore.get(), VK_NULL_HANDLE,
            m_device.GetPhysicalDevice(), m_device.GetDevice(), m_surface.GetSurface());
		swapChainImageIndex = acquireImageResult.imageIndex;

        if (acquireImageResult.needsRecreation) {
            m_renderImages.CreateResources(m_device, m_instance,m_queue, m_allocator, m_swapChain);
            m_bloomRes.onSwapchainExtentChanged(m_renderImages, m_device.GetDevice());
            acquireImageResult = m_swapChain.AcquireNextImage(frameSyncDataTemp.m_ImageAvaibleSemaphore.get(), VK_NULL_HANDLE,
                m_device.GetPhysicalDevice(), m_device.GetDevice(), m_surface.GetSurface());
            swapChainImageIndex = acquireImageResult.imageIndex;
		}
		RFCT_ASSERT(acquireImageResult.Succeeded()) // Failed to acquire swapchain image
        if (acquireImageResult.imageIndex == -1)
        {
            return;
        }
    }
    currentFrameFGResources.ResetFences(m_device.GetDevice());
    m_frameGraph.GetResources().PreFrame(graphicsCtx, world::getWorld().changeSceneEffectMultiplier);
    {

        RFCT_PROFILE_SCOPE("command buffers record");
        m_scenePass.RecordCommandBuffer(graphicsCtx, frameSyncDataTemp, m_renderImages.GetSceneImage(graphicsCtx.frameInFlightIndex).m_frameBuffer.get());
        m_bloomRes.blum(graphicsCtx, m_renderImages, m_swapChain, frameSyncDataTemp, m_renderImages.GetIntermediateClearRenderPass(), swapChainImageIndex);
        debugDraw::flush(graphicsCtx, frameSyncDataTemp, m_renderImages.GetSwapChainImage(swapChainImageIndex).m_frameBuffer.get(), m_renderImages.GetIntermediateRenderPass());
        m_UIPipeline.draw(graphicsCtx, m_swapChain, frameSyncDataTemp, m_renderImages.GetSwapChainImage(swapChainImageIndex).m_frameBuffer.get(), m_renderImages.GetUIRenderPass());
    }
    {
        RFCT_PROFILE_SCOPE("command buffer submissions");
        constexpr vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

        vk::SubmitInfo sceneSubmitInfo = frameSyncDataTemp.sceneSubmitInfo(RfctFrameContext);
        sceneSubmitInfo.pWaitDstStageMask = waitStages;
        m_queue.SubmitGraphics(sceneSubmitInfo);

        vk::SubmitInfo bloomSubmitInfo = frameSyncDataTemp.bloomSubmitInfo(RfctFrameContext);
        bloomSubmitInfo.pWaitDstStageMask = waitStages;
        m_queue.SubmitGraphics(bloomSubmitInfo);
        if (RfctFrameContext.renderDebugDraw) 
        {
            vk::SubmitInfo debugDrawSubmitInfo = frameSyncDataTemp.debugDrawSubmitInfo(RfctFrameContext);
            debugDrawSubmitInfo.pWaitDstStageMask = waitStages;
            m_queue.SubmitGraphics(debugDrawSubmitInfo);
        }

        vk::SubmitInfo uiSubmitInfo = frameSyncDataTemp.uiSubmitInfo(RfctFrameContext);
        uiSubmitInfo.pWaitDstStageMask = waitStages;
        m_queue.SubmitGraphics(uiSubmitInfo, frameSyncDataTemp.m_thisFrameRenderFinishedFence);
    }
    {
        RFCT_PROFILE_SCOPE("image present");
        vk::PresentInfoKHR presentInfo{};
        presentInfo.sType = vk::StructureType::ePresentInfoKHR;

        //RFCT_VULKAN_CHECK(m_device.GetDevice().waitForFences(1, &frameSyncDataTemp.m_thisFrameRenderFinishedFence, VK_TRUE, UINT64_MAX));

        presentInfo.waitSemaphoreCount = 1;
        const vk::Semaphore& sem = frameSyncDataTemp.m_renderFinishedSemaphore.get();
        presentInfo.pWaitSemaphores = &sem;

        presentInfo.swapchainCount = 1;
        vk::SwapchainKHR sc =  m_swapChain.GetSwapChain();
        presentInfo.pSwapchains = &sc;
        presentInfo.pImageIndices = &swapChainImageIndex;
        presentInfo.pResults = nullptr;

        vk::Result presRes = m_device.GetQueue().GetPresentQueue().presentKHR(&presentInfo);
        if (presRes == vk::Result::eSuboptimalKHR){
            m_swapChain.m_framebufferResized = true;
            RFCT_INFO("recreation needed");
        }else{
            if (presRes != vk::Result::eSuccess){
                RFCT_INFO("other present error");
            }
        }
    }
}
