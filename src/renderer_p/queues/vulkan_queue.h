#pragma once
#include <array>
#include <queue>
namespace rfct {
	std::array<uint32_t, 3> selectQueueFamilies(vk::PhysicalDevice physicalDevice);
	class vulkanQueueManager {
	public:
        vulkanQueueManager(vk::Device device, vk::PhysicalDevice physicalDevice);
        ~vulkanQueueManager();
        void submitGraphics(const vk::SubmitInfo& submitInfo, vk::Fence fence = nullptr);
        void submitCompute(const vk::SubmitInfo& submitInfo, vk::Fence fence = nullptr);
        void submitTransfer(const vk::SubmitInfo& submitInfo, vk::Fence fence = nullptr);

        void stop();

    private:
        void queueWorker(vk::Queue queue, std::queue<std::pair<vk::SubmitInfo, vk::Fence>>& taskQueue, std::mutex& queueMutex, std::condition_variable& condition, bool& running);

        vk::Device m_device;
        vk::Queue m_graphicsQueue;
        vk::Queue m_computeQueue;
        vk::Queue m_transferQueue;

        std::queue<std::pair<vk::SubmitInfo, vk::Fence>> m_graphicsTasks;
        std::queue<std::pair<vk::SubmitInfo, vk::Fence>> m_computeTasks;
        std::queue<std::pair<vk::SubmitInfo, vk::Fence>> m_transferTasks;

        std::mutex m_graphicsMutex;
        std::mutex m_computeMutex;
        std::mutex m_transferMutex;

        std::condition_variable m_graphicsCondition;
        std::condition_variable m_computeCondition;
        std::condition_variable m_transferCondition;

        std::thread m_graphicsThread;
        std::thread m_computeThread;
        std::thread m_transferThread;

        bool m_running = true;
	};
}