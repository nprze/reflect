#pragma once
#include "renderer_p\queues\vulkan_queue.h"
namespace rfct {
	vk::PhysicalDevice chooseBestPhysicalDevice();
	class vulkanDevice {
	public:
		static const std::array<const char*, 4> deviceRequiredExtensions;
	public:
		vk::Device getDevice() { return m_device.get(); }
	public:
		vulkanDevice();
	private:
		vk::PhysicalDevice m_physicalDevice;
		vk::UniqueDevice m_device;
		vulkanQueueManager m_queueManager;
		bool m_rayTracingSupported;
	};
}
