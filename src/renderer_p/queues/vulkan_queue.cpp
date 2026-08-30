#include "vulkan_queue.h"
#include <iostream>
#include <sstream>
#include "renderer_p/renderer.h"

uint32_t lastGraphicsQueueFamilyIndex = -1;


rfct::vulkanQueueManager::vulkanQueueManager(vk::Device device, vk::PhysicalDevice physicalDevice)
    : m_device(device) {
    RFCT_PROFILE_FUNCTION();
    RFCT_ASSERT(lastGraphicsQueueFamilyIndex != -1)
    m_graphicsQueueFamilyIndex = lastGraphicsQueueFamilyIndex;
    m_graphicsQueue = m_device.getQueue(lastGraphicsQueueFamilyIndex, 0);
}

void rfct::vulkanQueueManager::submitGraphics(const vk::SubmitInfo& submitInfo, vk::Fence fence) {
    RFCT_PROFILE_FUNCTION();
    m_graphicsQueue.submit(submitInfo, fence);
}