#include "renderer.h"

#include "job_system_p/job_system.h"
#include "frame/frame_data.h"
#include "world_p/scene.h"
#include "assets/assets_manager.h"

namespace rfct {
    renderer* renderer::ren = nullptr;
    bool setStaticRenderer(renderer *rendererArg) { // this exists bcs the static renderer var needs to be set before the components' constructors
        renderer::ren = rendererArg;
        return true;
    }

    // surface wrapper
    SurfaceWrapper::SurfaceWrapper(vk::SurfaceKHR surfaceArg) {
        surface = surfaceArg;
    }
    void SurfaceWrapper::newSurface(vk::SurfaceKHR surfaceArg){
        renderer::getRen().getInstance().destroySurfaceKHR(surface);
        surface = surfaceArg;
    }
    SurfaceWrapper::~SurfaceWrapper() {
        renderer::getRen().getInstance().destroySurfaceKHR(surface);
    }

    // vma allocator wrapper
    allocator::allocator()
    {
#ifdef ANDROID_BUILD
        VmaAllocatorCreateInfo allocatorCreateInfo = {};
        allocatorCreateInfo.physicalDevice = rfct::renderer::getRen().getDeviceWrapper().getPhysicalDevice();
        allocatorCreateInfo.device = rfct::renderer::getRen().getDevice();
        allocatorCreateInfo.instance = rfct::renderer::getRen().getInstance();
        allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        VmaVulkanFunctions vulkanFunctions = {};

        vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
        vulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
        vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
        vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
        allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

        vmaCreateAllocator(&allocatorCreateInfo, &m_allocator);
#else
        VmaAllocatorCreateInfo allocatorCreateInfo = {};
        allocatorCreateInfo.physicalDevice = rfct::renderer::getRen().getDeviceWrapper().getPhysicalDevice();
        allocatorCreateInfo.device = rfct::renderer::getRen().getDevice();
        allocatorCreateInfo.instance = rfct::renderer::getRen().getInstance();
        allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        vmaCreateAllocator(&allocatorCreateInfo, &m_allocator);
#endif
    }

    allocator::~allocator()
    {
        vmaDestroyAllocator(m_allocator);
    }

}


// LET THIS CODE COOK. IT DOES COOK FRFR
rfct::renderer::renderer(RFCT_RENDERER_ARGUMENTS)
	: m_uselessBool(setStaticRenderer(this)), 
    m_window(RFCT_WINDOWS_WINDOW_ARGUMENTS RFCT_NATIVE_WINDOW_ANDROID_VAR), 
    m_instance(), 
    m_surface(m_window.createSurface(getInstance())), 
    m_device(), 
    m_rasterizerPipeline(), 
    m_allocator(), 
    m_framesInFlight(), 
    m_debugDraw(),
    m_UIPipeline()
{
    m_device.getSwapChain().createMSAAres(msaaSamples);
    m_device.getSwapChain().createFrameBuffers();
}

void rfct::renderer::updateWindow(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR){ 
// surface holder change on android
#ifdef ANDROID_BUILD
    m_window.destroyWind();
    m_window = AndroidWindow(RFCT_NATIVE_WINDOW_ANDROID_VAR);
    m_surface.newSurface(m_window.createSurface(getInstance()));
#endif // ANDROID_BUILD
};

rfct::renderer::~renderer() {
    AssetsManager::get().cleanup();
    m_device.getSwapChain().cleanupMSAAres();
};

void rfct::renderer::render(frameContext& frameContext)
{
    RFCT_PROFILE_FUNCTION(); 
	frameData& frameData = m_framesInFlight.getNextFrame(frameContext.frame);
    {
        RFCT_PROFILE_SCOPE("fences wait");
        frameData.waitForFences();
    }

    uint32_t imageIndex;
    {
        RFCT_PROFILE_SCOPE("get sawpchain image");
        imageIndex = m_device.getSwapChain().acquireNextImage(frameData.m_ImageAvaibleSemaphore.get(), VK_NULL_HANDLE);

        if (imageIndex == -1)
        {
            return;
        }
    }
    frameData.resetFences();
    frameData.prepareFrame(frameContext.frame);
    {
        RFCT_PROFILE_SCOPE("command buffers record");
        auto jobs = std::make_shared<rfct::jobTracker>();
        jobSystem::get().KickJob([&]() {
            m_rasterizerPipeline.recordCommandBuffer(&frameContext, frameData, m_device.getSwapChain().getSceneFrameBuffer(imageIndex));
            }, *jobs);
        jobSystem::get().KickJob([&]() {
            debugDraw::flush(&frameContext, frameData, m_device.getSwapChain().getUIFrameBuffer(imageIndex));
            }, *jobs);
        jobSystem::get().KickJob([&]() {
            m_UIPipeline.draw(frameData, m_device.getSwapChain().getUIFrameBuffer(imageIndex));
            }, *jobs);
        jobs->waitAll();
    }
    {
        RFCT_PROFILE_SCOPE("command buffer submissions");
        constexpr vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

        vk::SubmitInfo sceneSubmitInfo = frameData.sceneSubmitInfo(frameContext);
        sceneSubmitInfo.pWaitDstStageMask = waitStages;
        renderer::getRen().getDeviceWrapper().getQueueManager().submitGraphics(sceneSubmitInfo);

        if (frameContext.renderDebugDraw) {
            vk::SubmitInfo debugDrawSubmitInfo = frameData.debugDrawSubmitInfo(frameContext);
            debugDrawSubmitInfo.pWaitDstStageMask = waitStages;
            renderer::getRen().getDeviceWrapper().getQueueManager().submitGraphics(debugDrawSubmitInfo);
        }

        vk::SubmitInfo uiSubmitInfo = frameData.uiSubmitInfo(frameContext);
        uiSubmitInfo.pWaitDstStageMask = waitStages;
         renderer::getRen().getDeviceWrapper().getQueueManager().submitGraphics(uiSubmitInfo, frameData.m_thisFrameRenderFinishedFence);
    }




    {
        RFCT_PROFILE_SCOPE("image present");
        vk::PresentInfoKHR presentInfo{};
        presentInfo.sType = vk::StructureType::ePresentInfoKHR;

        RFCT_VULKAN_CHECK(m_device.getDevice().waitForFences(1, &frameData.m_lastFrameRenderFinishedFence, VK_TRUE, UINT64_MAX));

        presentInfo.waitSemaphoreCount = 1;
        const vk::Semaphore& sem = frameData.m_renderFinishedSemaphore.get();
        presentInfo.pWaitSemaphores = &sem;

        presentInfo.swapchainCount = 1;
        vk::SwapchainKHR sc = m_device.getSwapChain().getSwapChain();
        presentInfo.pSwapchains = &sc;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;
        RFCT_VULKAN_CHECK(m_device.getQueueManager().getPresentQueue().presentKHR(&presentInfo));
    }
}

void rfct::renderer::setObjectName(void* objectHandle, const std::string& name, vk::ObjectType objectType)
{
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
