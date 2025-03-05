#include "vulkan_queue.h"
#include "vulkan/vulkan.hpp"
#include <iostream>
#include <sstream>
#include <set>
#include <string>
#include "renderer_p\renderer.h"

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
#ifdef VK_ENABLE_BETA_EXTENSIONS
    if (queueFlags & vk::QueueFlagBits::eVideoDecodeKHR) {
        flagNames.push_back("Video Decode");
    }
    if (queueFlags & vk::QueueFlagBits::eVideoEncodeKHR) {
        flagNames.push_back("Video Encode");
    }
#endif
#ifdef VK_NV_OPTICAL_FLOW_EXTENSION_NAME
    if (queueFlags & vk::QueueFlagBits::eOpticalFlowNV) {
        flagNames.push_back("Optical Flow");
    }
#endif

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

std::array<uint32_t, 3> rfct::selectQueueFamilies(vk::PhysicalDevice physicalDevice) {
    std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

    std::tuple<int, uint32_t> graphicsAndPresentFamily = { -1, 0 };
    std::tuple<int, uint32_t> computeFamily = { -1, 0 };
    std::tuple<int, uint32_t> transferFamily = { -1, 0 };

    for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
        uint32_t queueCount = queueFamilies[i].queueCount;
        vk::QueueFlags flags = queueFamilies[i].queueFlags;
        bool supportsPresent = physicalDevice.getSurfaceSupportKHR(i, renderer::getRen().getInstanceWrapper().getSurface());

        if ((flags & vk::QueueFlagBits::eGraphics) && supportsPresent) {
            if (queueCount > std::get<1>(graphicsAndPresentFamily)) {
                graphicsAndPresentFamily = { i, queueCount };
            }
        }
        else if ((flags & vk::QueueFlagBits::eCompute) && !(flags & vk::QueueFlagBits::eGraphics)) {
            if (queueCount > std::get<1>(computeFamily)) {
                computeFamily = { i, queueCount };
            }
        }
        else if ((flags & vk::QueueFlagBits::eTransfer) && !(flags & vk::QueueFlagBits::eGraphics) && !(flags & vk::QueueFlagBits::eCompute)) {
            if (queueCount > std::get<1>(transferFamily)) {
                transferFamily = { i, queueCount };
            }
        }
    }

    return {
        static_cast<uint32_t>(std::get<0>(graphicsAndPresentFamily)),
        static_cast<uint32_t>(std::get<0>(computeFamily)),
        static_cast<uint32_t>(std::get<0>(transferFamily))
    };
}

rfct::vulkanQueueManager::vulkanQueueManager(vk::Device device, vk::PhysicalDevice physicalDevice)
    : m_device(device) {
    auto queueFamilies = selectQueueFamilies(physicalDevice);

    m_graphicsQueueFamilyIndex = queueFamilies[0];
    m_computeQueueFamilyIndex = queueFamilies[1];
    m_transferQueueFamilyIndex = queueFamilies[2];

    m_graphicsQueue = m_device.getQueue(queueFamilies[0], 0);
    m_computeQueue = m_device.getQueue(queueFamilies[1], 0);
    m_transferQueue = m_device.getQueue(queueFamilies[2], 0);
}

rfct::vulkanQueueManager::~vulkanQueueManager() {
    // No threads to stop, so nothing to do here.
}

void rfct::vulkanQueueManager::submitGraphics(const vk::SubmitInfo& submitInfo, vk::Fence fence) {
    // Directly submit to the graphics queue
    m_graphicsQueue.submit(submitInfo, fence);
}

void rfct::vulkanQueueManager::submitCompute(const vk::SubmitInfo& submitInfo, vk::Fence fence) {
    // Directly submit to the compute queue
    m_computeQueue.submit(submitInfo, fence);
}

void rfct::vulkanQueueManager::submitTransfer(const vk::SubmitInfo& submitInfo, vk::Fence fence) {
    // Directly submit to the transfer queue
    m_transferQueue.submit(submitInfo, fence);
}