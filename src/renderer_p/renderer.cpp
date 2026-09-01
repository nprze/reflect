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

rfct::RfctRenderer::RfctRenderer(RFCT_RENDERER_ARGUMENTS)
	: m_window(RFCT_WINDOWS_WINDOW_ARGUMENTS RFCT_NATIVE_WINDOW_ANDROID_VAR),
    m_instance(), 
    m_surface(m_window.CreateSurface(m_instance.GetInstance())),
    m_device(m_instance.GetInstance(), m_surface.GetSurface()), 
    m_allocator(m_device.GetPhysicalDevice(), m_device.GetDevice(), m_instance.GetInstance()),
	m_swapChain(m_device.GetPhysicalDevice(), m_device.GetDevice(), m_surface.GetSurface()),
    m_renderImages(),
    m_rasterizerPipeline(m_renderImages.getSceneRenderPass()), 
    m_framesInFlight(), 
    m_bloomRes(m_renderImages.getIntermediateRenderPass()),
    m_debugDraw(m_renderImages.getIntermediateRenderPass()),
    m_UIPipeline(m_renderImages.getUIRenderPass())
{
}

rfct::RfctRenderer::~RfctRenderer() {
    cleanupAssetsCommandPool();
};

void rfct::RfctRenderer::UpdateWindow(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR) {
    RFCT_PROFILE_FUNCTION();
    // surface holder change on android
#ifdef ANDROID_BUILD
    m_device.getDevice().waitIdle();
    if (m_surface.surface) {
        m_instance.getInstance().destroySurfaceKHR(m_surface.surface);
        m_surface.surface = VK_NULL_HANDLE;
    }
    m_window.destroyWind();
    m_window = AndroidWindow(RFCT_NATIVE_WINDOW_ANDROID_VAR);
    m_surface.newSurface(m_window.createSurface(getInstance()));
    RFCT_WARN("destroying widnow and surface. creating new surface with width, hwight: ({}, {})", m_window.getExtent().width, m_window.getExtent().height);
#endif
};

void rfct::RfctRenderer::Render(frameContext& frameContext) {
    RFCT_PROFILE_FUNCTION();
	frameData& frameData = m_framesInFlight.getNextFrame(frameContext.frame);
    {
        RFCT_PROFILE_SCOPE("fences wait");
        frameData.waitForFences();
    }

    uint32_t imageIndex;
    {
        RFCT_PROFILE_SCOPE("get sawpchain image");
        imageIndex = m_renderImages.acquireNextImage(frameData.m_ImageAvaibleSemaphore.get(), VK_NULL_HANDLE);

        if (imageIndex == -1)
        {
            return;
        }
    }
    frameData.resetFences();
    frameData.prepareFrame(frameContext, frameContext.frame, world::getWorld().changeSceneEffectMultiplier);
    {
        RFCT_PROFILE_SCOPE("command buffers record");
        auto jobs = std::make_shared<rfct::jobTracker>();
        jobSystem::get().KickJob([&]() {
            m_rasterizerPipeline.recordCommandBuffer(&frameContext, frameData, m_renderImages.getSceneFrameBuffer(frameContext.frame), m_renderImages.getSceneRenderPass());
            }, *jobs);
        jobSystem::get().KickJob([&]() {
            m_bloomRes.blum(&frameContext, frameData, m_renderImages.getIntermediateClearRenderPass(), imageIndex);
            }, *jobs);
        jobSystem::get().KickJob([&]() {
            debugDraw::flush(&frameContext, frameData, m_renderImages.getSwapChainFrameBuffer(imageIndex), m_renderImages.getIntermediateRenderPass());
            }, *jobs);
        jobSystem::get().KickJob([&]() {
            m_UIPipeline.draw(frameData, m_renderImages.getSwapChainFrameBuffer(imageIndex), m_renderImages.getUIRenderPass());
            }, *jobs);
        jobs->waitAll();
    }
    {
        RFCT_PROFILE_SCOPE("command buffer submissions");
        constexpr vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

        vk::SubmitInfo sceneSubmitInfo = frameData.sceneSubmitInfo(frameContext);
        sceneSubmitInfo.pWaitDstStageMask = waitStages;
        RfctRenderer::getRen().getDeviceWrapper().GetQueue().submitGraphics(sceneSubmitInfo);

        vk::SubmitInfo bloomSubmitInfo = frameData.bloomSubmitInfo(frameContext);
        bloomSubmitInfo.pWaitDstStageMask = waitStages;
        RfctRenderer::getRen().getDeviceWrapper().GetQueue().submitGraphics(bloomSubmitInfo);

        if (frameContext.renderDebugDraw) 
        {
            vk::SubmitInfo debugDrawSubmitInfo = frameData.debugDrawSubmitInfo(frameContext);
            debugDrawSubmitInfo.pWaitDstStageMask = waitStages;
            RfctRenderer::getRen().getDeviceWrapper().GetQueue().submitGraphics(debugDrawSubmitInfo);
        }

        vk::SubmitInfo uiSubmitInfo = frameData.uiSubmitInfo(frameContext);
        uiSubmitInfo.pWaitDstStageMask = waitStages;
         RfctRenderer::getRen().getDeviceWrapper().GetQueue().submitGraphics(uiSubmitInfo, frameData.m_thisFrameRenderFinishedFence);
    }
    {
        RFCT_PROFILE_SCOPE("image present");
        vk::PresentInfoKHR presentInfo{};
        presentInfo.sType = vk::StructureType::ePresentInfoKHR;

        RFCT_VULKAN_CHECK(m_device.getDevice().waitForFences(1, &frameData.m_thisFrameRenderFinishedFence, VK_TRUE, UINT64_MAX));

        presentInfo.waitSemaphoreCount = 1;
        const vk::Semaphore& sem = frameData.m_renderFinishedSemaphore.get();
        presentInfo.pWaitSemaphores = &sem;

        presentInfo.swapchainCount = 1;
        vk::SwapchainKHR sc =  m_renderImages.getSwapChain().getSwapChain();
        presentInfo.pSwapchains = &sc;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        vk::Result presRes = m_device.GetQueue().getPresentQueue().presentKHR(&presentInfo);
        if (presRes == vk::Result::eSuboptimalKHR){
            getRenderImagesManager().getSwapChain().framebufferResized = true;
            RFCT_INFO("recreation needed");
        }else{
            if (presRes != vk::Result::eSuccess){
                RFCT_INFO("other present error");

            }
        }
    }
}

void rfct::RfctRenderer::SetObjectName(void* objectHandle, const std::string& name, vk::ObjectType objectType) {
    RFCT_PROFILE_FUNCTION();
#ifndef RFCT_VULKAN_DEBUG_OFF
    if (!m_instance.getDynamicLoader().vkSetDebugUtilsObjectNameEXT) {
        RFCT_CRITICAL("Failed to load vkSetDebugUtilsObjectNameEXT!");
    }

    vk::DebugUtilsObjectNameInfoEXT nameInfo{};
    nameInfo.objectType = objectType;
    nameInfo.objectHandle = (uintptr_t)(objectHandle);
    nameInfo.pObjectName = name.c_str();

    m_device.getDevice().setDebugUtilsObjectNameEXT(nameInfo, m_instance.getDynamicLoader());
#endif // RFCT_VULKAN_DEBUG_OFF
}
