#pragma once
#include "renderer_p/queues/vulkan_queue.h"

namespace rfct {
	vk::PhysicalDevice chooseBestPhysicalDevice();
	class vulkanDevice {
	public:
		static const std::array<const char*, 1> deviceRequiredExtensions;
	public:
		vk::Device& getDevice() { return m_device.get(); }
		inline vulkanQueueManager& getQueueManager() { return m_queueManager; }
		vk::PhysicalDevice& getPhysicalDevice() { return m_physicalDevice; }
	private:
		vulkanDevice();
	private:
		vk::PhysicalDevice m_physicalDevice;
		vk::UniqueDevice m_device;
		vulkanQueueManager m_queueManager;

		friend class renderer;
	};
}
