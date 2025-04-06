#pragma once
#include <array>
namespace rfct {
    std::array<uint32_t, 3> selectQueueFamilies(vk::PhysicalDevice physicalDevice);

    class vulkanQueueManager {
    public:
        vulkanQueueManager(vk::Device device, vk::PhysicalDevice physicalDevice);
        ~vulkanQueueManager();

        // Directly submit tasks to the respective queues
        void submitGraphics(const vk::SubmitInfo& submitInfo, vk::Fence fence = nullptr);
        void submitCompute(const vk::SubmitInfo& submitInfo, vk::Fence fence = nullptr);
        void submitTransfer(const vk::SubmitInfo& submitInfo, vk::Fence fence = nullptr);
        inline vk::Queue getPresentQueue() { return m_graphicsQueue; }
        uint32_t getGraphicsQueueFamilyIndex() { return m_graphicsQueueFamilyIndex; }

    private:
        // Removed worker function, as threading is no longer needed

        vk::Device m_device;
        vk::Queue m_graphicsQueue;
        vk::Queue m_computeQueue;
        vk::Queue m_transferQueue;

        uint32_t m_graphicsQueueFamilyIndex;
        uint32_t m_computeQueueFamilyIndex;
        uint32_t m_transferQueueFamilyIndex;

        std::mutex m_graphicsMutex;
    };
}