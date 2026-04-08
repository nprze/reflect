#include "vulkan_queue.h"
#include <iostream>
#include <sstream>
#include "renderer_p/renderer.h"

uint32_t lastGraphicsQueueFamilyIndex = -1;

// Helper function
std::string queueFlagsToString(vk::QueueFlags queueFlags) {
    std::vector<std::string> flagNames;

    if (queueFlags & vk::QueueFlagBits::eGraphics) {
        flagNames.push_back("Graphics");
    }
    if (queueFlags & vk::QueueFlagBits::eCompute) {
        flagNames.push_back("Compute");
    }
    if (queueFlags & vk::QueueFlagBits::eTransfer) {
        flagNames.push_back("Transfer");
    }
    if (queueFlags & vk::QueueFlagBits::eSparseBinding) {
        flagNames.push_back("Sparse Binding");
    }
    if (queueFlags & vk::QueueFlagBits::eProtected) {
        flagNames.push_back("Protected");
    }

    if (flagNames.empty()) {
        return "{}";
    }

    std::ostringstream oss;
    oss << "{ ";
    for (size_t i = 0; i < flagNames.size(); ++i) {
        oss << flagNames[i];
        if (i != flagNames.size() - 1) {
            oss << " | ";
        }
    }
    oss << " }";

    return oss.str();
}

uint32_t rfct::selectQueueFamily(vk::PhysicalDevice physicalDevice) {
    RFCT_PROFILE_FUNCTION();
    std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

    std::pair<int, uint32_t> graphicsAndPresentFamily = { -1, 0 };
    std::pair<int, uint32_t> computeFamily = { -1, 0 };
    std::pair<int, uint32_t> transferFamily = { -1, 0 };

    // give priority to queue families with more queue count
    for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
        uint32_t queueCount = queueFamilies[i].queueCount;
        vk::QueueFlags flags = queueFamilies[i].queueFlags;
        RFCT_INFO("queue {}, flags {}", i, queueFlagsToString(flags));
        bool supportsPresent = physicalDevice.getSurfaceSupportKHR(i, renderer::getRen().getSurface());

        if ((flags & vk::QueueFlagBits::eGraphics) && supportsPresent) {
            if (queueCount > (graphicsAndPresentFamily.second)) {
                graphicsAndPresentFamily = { i, queueCount };
            }
        }
        else if ((flags & vk::QueueFlagBits::eCompute) && !(flags & vk::QueueFlagBits::eGraphics)) {
            if (queueCount > (computeFamily.second)) {
                computeFamily = { i, queueCount };
            }
        }
        else if ((flags & vk::QueueFlagBits::eTransfer) && !(flags & vk::QueueFlagBits::eGraphics) && !(flags & vk::QueueFlagBits::eCompute)) {
            if (queueCount > (transferFamily.second)) {
                transferFamily = { i, queueCount };
            }
        }
    }
	lastGraphicsQueueFamilyIndex = graphicsAndPresentFamily.first;
    return static_cast<uint32_t>(graphicsAndPresentFamily.first);
}

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