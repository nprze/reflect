#pragma once
#include <vulkan/vulkan.hpp>
#include "renderer_p/queues/vulkan_queue.h"

namespace rfct {
	class RfctQueue {
	public:
		void submitGraphics(const vk::SubmitInfo& submitInfo, vk::Fence fence = nullptr); // submit only on main thread
		inline vk::Queue getPresentQueue() { return m_graphicsQueue; }
		uint32_t getGraphicsQueueFamilyIndex() { return m_graphicsQueueFamilyIndex; }
	private:
		RfctQueue(vk::Device device, vk::PhysicalDevice physicalDevice);
	private:
		vk::Device m_device;
		vk::Queue m_graphicsQueue;
		uint32_t m_graphicsQueueFamilyIndex;
	};

	class RfctDevice {
	public:
		static const std::array<const char*, 1> deviceRequiredExtensions;
	public:
		vk::Device& getDevice() { return m_device.get(); }
		inline RfctQueue& getQueueManager() { return m_queueManager; }
		vk::PhysicalDevice& getPhysicalDevice() { return m_physicalDevice; }
	private:
		RfctDevice();
	private:
		vk::PhysicalDevice m_physicalDevice;
		vk::UniqueDevice m_device;
		RfctQueue m_queueManager;
	};

	class RfctVulkanInstance {
	public:
		vk::Instance& getInstance() { return m_instance.get(); }
		RFCT_VULKAN_INSTANCE_NAMESPACE DispatchLoaderDynamic& getDynamicLoader() { return m_dynamicLoader; }
	private:
		RfctVulkanInstance();
	private:
		vk::UniqueInstance m_instance;
		vk::UniqueHandle<vk::DebugUtilsMessengerEXT, RFCT_VULKAN_INSTANCE_NAMESPACE DispatchLoaderDynamic> m_debugMessenger;
		RFCT_VULKAN_INSTANCE_NAMESPACE DispatchLoaderDynamic m_dynamicLoader;
	};
}
