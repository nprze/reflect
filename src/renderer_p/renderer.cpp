#include "renderer.h"
#include "job_system_p\job_system.h"
#include <stdint.h>
#include "frame\frame_data.h"
#include "world_p\scene.h"

namespace rfct {
    AssetsManager* setStaticRenderer(renderer *rendererArg, AssetsManager* assetsManager) {
        renderer::ren = rendererArg;
        return assetsManager;
    }

    renderer *renderer::ren = nullptr;
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
}


// LET THIS CODE COOK. IT DOES COOK FRFR
rfct::renderer::renderer(RFCT_RENDERER_ARGUMENTS)
	: m_AssetsManager(setStaticRenderer(this, assetsManager)), m_window(RFCT_WINDOWS_WINDOW_ARGUMENTS RFCT_NATIVE_WINDOW_ANDROID_VAR), m_instance(), m_surface(m_window.createSurface(getInstance())), m_device(), m_AssetsCommandPool(m_device.getDevice().createCommandPoolUnique({ {}, m_device.getQueueManager().getGraphicsQueueFamilyIndex() })), m_rasterizerPipeline(), m_allocator(), m_framesInFlight(), m_debugDraw()
{
    m_device.getSwapChain().createFrameBuffers();
}

void rfct::renderer::updateWindow(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR){
#ifdef ANDROID_BUILD
    m_window.destroyWind();
    m_window = AndroidWindow(RFCT_NATIVE_WINDOW_ANDROID_VAR);

    m_surface.newSurface(m_window.createSurface(getInstance()));

#endif // ANDROID_BUILD

};

rfct::renderer::~renderer() {
    m_device.getDevice().destroyDescriptorSetLayout(cameraUbo::getDescriptorSetLayout());
};

void rfct::renderer::showWindow()
{
	m_window.show();
}

void rfct::renderer::render(frameContext& frameContext)
{
    RFCT_PROFILE_FUNCTION(); 
	frameData& frame = m_framesInFlight.getNextFrame(&frameContext);
    {
        RFCT_PROFILE_SCOPE("fences wait");
        frame.waitForAllFences();
    }

    uint32_t imageIndex;
    {
        RFCT_PROFILE_SCOPE("get sawpchain image");
        imageIndex = m_device.getSwapChain().acquireNextImage(frame.m_ImageAvaibleSemaphore.get(), VK_NULL_HANDLE);

        if (imageIndex == -1)
        {
            return;
        }
        RFCT_MARK("acquired frame");
    }
    //RFCT_ASSERT(imageIndex == frameContext.frame);
    frame.resetAllFences();
    frame.prepareFrame(frameContext.frame);
    auto jobs = std::make_shared<rfct::jobTracker>();
    jobSystem::get().KickJob([&]() {
      m_rasterizerPipeline.recordCommandBuffer(&frameContext, frameContext.scene->getRenderData(), frame, m_device.getSwapChain().getFrameBuffer(imageIndex), imageIndex);
    }, *jobs);
    
    jobSystem::get().KickJob([&]() {
      debugDraw::flush(&frameContext, frame, m_device.getSwapChain().getFrameBuffer(imageIndex), imageIndex);
    }, *jobs);
    jobSystem::get().KickJob([&]() {
     m_UIPipeline.draw(frame, m_device.getSwapChain().getFrameBuffer(imageIndex), imageIndex);
    }, *jobs);
	jobs->waitAll();





    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };


    vk::SubmitInfo sceneSubmitInfo = frame.sceneSubmitInfo(frameContext);
    sceneSubmitInfo.pWaitDstStageMask = waitStages; 
    renderer::getRen().getDeviceWrapper().getQueueManager().submitGraphics(sceneSubmitInfo);

    if (frameContext.renderDebugDraw) {
        vk::SubmitInfo debugDrawSubmitInfo = frame.debugDrawSubmitInfo(frameContext);
        debugDrawSubmitInfo.pWaitDstStageMask = waitStages;
        renderer::getRen().getDeviceWrapper().getQueueManager().submitGraphics(debugDrawSubmitInfo);
    }

    vk::SubmitInfo uiSubmitInfo = frame.uiSubmitInfo(frameContext);
    uiSubmitInfo.pWaitDstStageMask = waitStages;
    renderer::getRen().getDeviceWrapper().getQueueManager().submitGraphics(uiSubmitInfo, frame.m_thisFrameRenderFinishedFence);





    vk::PresentInfoKHR presentInfo{};
    presentInfo.sType = vk::StructureType::ePresentInfoKHR;

    RFCT_VULKAN_CHECK(m_device.getDevice().waitForFences(1, &frame.m_lastFrameRenderFinishedFence, VK_TRUE, UINT64_MAX));

    presentInfo.waitSemaphoreCount = 1;
    const vk::Semaphore& sem = frame.m_renderFinishedSemaphore.get();
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

rfct::allocator::allocator()
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

rfct::allocator::~allocator()
{
	vmaDestroyAllocator(m_allocator);
}
