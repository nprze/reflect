#pragma once

namespace rfct {
    uint32_t selectQueueFamily(vk::PhysicalDevice physicalDevice);
    class vulkanQueueManager {
    public:
        void submitGraphics(const vk::SubmitInfo& submitInfo, vk::Fence fence = nullptr); // submit only on main thread
        inline vk::Queue getPresentQueue() { return m_graphicsQueue; }
        uint32_t getGraphicsQueueFamilyIndex() { return m_graphicsQueueFamilyIndex; }
    private:
        vulkanQueueManager(vk::Device device, vk::PhysicalDevice physicalDevice);
    private:
        vk::Device m_device;
        vk::Queue m_graphicsQueue;
        uint32_t m_graphicsQueueFamilyIndex;

        friend class vulkanDevice;
    };
}