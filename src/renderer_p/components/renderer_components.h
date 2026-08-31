#pragma once
#include <vulkan/vulkan.hpp>
#include "renderer_p/queues/vulkan_queue.h"
#include "platform_window.h"

namespace rfct {
	class RfctQueue {
	public:
		inline vk::Queue getPresentQueue() { return m_graphicsQueue; }
		inline uint32_t getGraphicsQueueFamilyIndex() { return m_graphicsQueueFamilyIndex; }
	public:
		void SubmitGraphics(const vk::SubmitInfo& submitInfo, vk::Fence fence = nullptr); // submit only on main thread
		RfctQueue(vk::Device device, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);
	private:
		vk::Device m_device;
		vk::Queue m_graphicsQueue;
		uint32_t m_graphicsQueueFamilyIndex;
	};

	class RfctDevice {
	public:
		inline vk::Device& GetDevice() { return m_device.get(); }
		inline RfctQueue& GetQueue() { return m_queue; }
		inline vk::PhysicalDevice& GetPhysicalDevice() { return m_physicalDevice; }
	public:
		RfctDevice(vk::Instance instance, vk::SurfaceKHR surface);
	private:
		vk::PhysicalDevice m_physicalDevice;
		vk::UniqueDevice m_device;
		RfctQueue m_queue;
	};

	class RfctVulkanInstance {
	public:
		vk::Instance& GetInstance() { return m_instance.get(); }
		RFCT_VULKAN_LOADER_NAMESPACE::DispatchLoaderDynamic& GetDynamicLoader() { return m_dynamicLoader; }
	public:
		RfctVulkanInstance();
	private:
		vk::UniqueInstance m_instance;
		bool m_debugEnabled = false;
		vk::UniqueHandle<vk::DebugUtilsMessengerEXT, RFCT_VULKAN_LOADER_NAMESPACE::DispatchLoaderDynamic> m_debugMessenger;
		RFCT_VULKAN_LOADER_NAMESPACE::DispatchLoaderDynamic m_dynamicLoader;
	};

	class RfctSwapChain {
	public:
		void CreateSwapChain(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface);
		void RecreateSwapChain(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface);
		uint32_t AcquireNextImage(const vk::Semaphore& semaphore, vk::Fence fence, vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface); // will recreate swapchain if unoptimal
		vk::SwapchainKHR GetSwapChain() { return m_swapChain.get(); }
		vk::SurfaceFormatKHR GetSurfaceFormat() { return m_surfaceFormat; }
		vk::Extent2D GetExtent() { return m_swapChainExtent; }
	private:
		RfctSwapChain(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface);
	public:
		bool framebufferResized = false;
	private:
		vk::UniqueSwapchainKHR m_swapChain;
		vk::Extent2D m_swapChainExtent;
		vk::SurfaceFormatKHR m_surfaceFormat;
	};
}
