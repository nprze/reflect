#include "renderer.h"
#include <stdint.h>


namespace rfct {
    uselessClass createUselessClass(renderer *rendererArg) {
        renderer::ren = rendererArg;
        return {false};
    }

    renderer *renderer::ren = nullptr;
}
// LET THIS CODE COOK. IT DOES COOK FRFR
rfct::renderer::renderer(RFCT_RENDERER_ARGUMENTS)
	: m_AssetsManager(assetsManager), uc(createUselessClass(this)), m_window(RFCT_WINDOWS_WINDOW_ARGUMENTS RFCT_NATIVE_WINDOW_ANDROID_VAR), m_instance(), m_device(), m_rasterizerPipeline(), m_allocator(), m_framesInFlight(), m_rayTracer(), m_vertexBuffer(sizeof(Vertex) * 3), m_debugDraw()
{
    std::vector<Vertex> vertices = {
        {{0.0f, -0.5f, 0.f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f}}
    };
	m_vertexBuffer.copyData(vertices);


    m_device.getSwapChain().createFrameBuffers();
}

rfct::renderer::~renderer() {
    m_device.getDevice().waitIdle(); 
};

void rfct::renderer::showWindow()
{
	m_window.show();
}

void rfct::renderer::render()
{
    RFCT_PROFILE_FUNCTION(); 
	frameData& frame = m_framesInFlight.getNextFrame();
    {
        RFCT_PROFILE_SCOPE("fences wait");
        frame.waitForAllFences();
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
    frame.resetAllFences();

	m_rasterizerPipeline.recordCommandBuffer(frame, m_device.getSwapChain().getFrameBuffer(imageIndex), imageIndex);
    
    debugDraw::flush(frame, m_device.getSwapChain().getFrameBuffer(imageIndex), imageIndex);
    frame.waitForAllFences();


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
