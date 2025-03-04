#include "renderer.h"
#include <stdint.h>

#ifdef WIN32
#include "windows\glfw_window.h"
#else
#include <android/native_window.h>
#include "android\android_window.h"
#endif

namespace rfct {
    uselessClass createUselessClass(renderer* rendererArg){
        renderer::ren = rendererArg;
        return {false};
    }
    renderer* renderer::ren = nullptr;

    /*
	shared<windowAbstact> createWindow() {
#ifdef WIN32
        return std::make_shared<GlfwWindow>(968, 422, "reflect");
#endif // WIN32
#ifdef ANDROID_BUILD
        return std::make_shared<AndroidWindow>(968, 422, "reflect");

#endif // ANDROID_BUILD
        RFCT_CRITICAL("build for either windows or android");
        return nullptr;

	}
*/
}
rfct::renderer::renderer(ANativeWindow* window)
	:  uc(createUselessClass(this)), m_window(window), m_instance(), m_device(), m_rasterizerPipeline(), m_allocator(), m_framesInFlight(), m_rayTracer(), m_vertexBuffer(sizeof(Vertex) * 3)
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

    RFCT_VULKAN_CHECK(m_device.getDevice().resetFences(1, &frame.m_inRenderFence.get()));
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

rfct::allocator::allocator()
{
    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.physicalDevice = rfct::renderer::ren->getDeviceWrapper().getPhysicalDevice();
    allocatorCreateInfo.device = rfct::renderer::ren->getDevice();
    allocatorCreateInfo.instance = rfct::renderer::ren->getInstance();
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    VmaVulkanFunctions vulkanFunctions = {};

    vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    vulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
    vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    vmaCreateAllocator(&allocatorCreateInfo, &m_allocator);
}

rfct::allocator::~allocator()
{
	vmaDestroyAllocator(m_allocator);
}
