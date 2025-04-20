#pragma once
#include "renderer_p/queues/vulkan_queue.h"
#include "renderer_p/swap_chain/vulkan_swap_chain.h"
namespace rfct {
	vk::PhysicalDevice chooseBestPhysicalDevice();
	class vulkanDevice {
	public:
		static const std::array<const char*, 1> deviceRequiredExtensions;
	public:
		vk::Device& getDevice() { return m_device.get(); }
		vulkanSwapChain& getSwapChain() { return m_swapChain; }
		inline vulkanQueueManager& getQueueManager() { return m_queueManager; }
		vk::PhysicalDevice& getPhysicalDevice() { return m_physicalDevice; }
	private:
		vk::PhysicalDevice m_physicalDevice;
		vk::UniqueDevice m_device;
		vulkanQueueManager m_queueManager;
		vulkanSwapChain m_swapChain;
		bool m_rayTracingSupported;
	private:
		vulkanDevice();
		friend class renderer;
	};
}
