#pragma once
namespace rfct {
    std::array<uint32_t, 3> selectQueueFamilies(vk::PhysicalDevice physicalDevice);

    class vulkanQueueManager {
    public:

        void submitGraphics(const vk::SubmitInfo& submitInfo, vk::Fence fence = nullptr); // submit only on main thread
        void submitCompute(const vk::SubmitInfo& submitInfo, vk::Fence fence = nullptr); // submit only on main thread
        void submitTransfer(const vk::SubmitInfo& submitInfo, vk::Fence fence = nullptr); // submit only on main thread

        inline vk::Queue getPresentQueue() { return m_graphicsQueue; }
        uint32_t getGraphicsQueueFamilyIndex() { return m_graphicsQueueFamilyIndex; }

    private:
        vk::Device m_device;
        vk::Queue m_graphicsQueue;
        vk::Queue m_computeQueue;
        vk::Queue m_transferQueue;

        uint32_t m_graphicsQueueFamilyIndex;
        uint32_t m_computeQueueFamilyIndex;
        uint32_t m_transferQueueFamilyIndex;

        vulkanQueueManager(vk::Device device, vk::PhysicalDevice physicalDevice);
        ~vulkanQueueManager();
        friend class vulkanDevice;
    };
}