#include "vulkan_queue.h"
#include "vulkan/vulkan.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <string>

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

void rfct::showQueueFamilies(VkPhysicalDevice device)
{/*
    std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

    EG_TRACE("The queue families:");
    for (const auto& queueFamily : queueFamilies) {
        EG_TRACE("queue number {0}", queueFamily.queueCount);
        EG_TRACE("queue family properties: {0}", queueFlagsToString(queueFamily.queueFlags));
    }*/
}
