#pragma once
#include <vulkan/vulkan.hpp>
#include "platform_window.h"

namespace rfct {
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

	class RfctDevice {
	public:
		vk::Device& GetDevice() { return m_device.get(); }
		RfctQueue& GetQueue() { return m_queue; }
		vk::PhysicalDevice& GetPhysicalDevice() { return m_physicalDevice; }
	public:
		RfctDevice(vk::Instance instance, vk::SurfaceKHR surface);
	private:
		vk::PhysicalDevice m_physicalDevice;
		vk::UniqueDevice m_device;
		RfctQueue m_queue;
	};

	class RfctQueue {
	public:
		vk::Queue getPresentQueue() { return m_graphicsQueue; }
		uint32_t getGraphicsQueueFamilyIndex() { return m_graphicsQueueFamilyIndex; }
	public:
		RfctQueue(vk::Device device, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);
		void SubmitGraphics(const vk::SubmitInfo& submitInfo, vk::Fence fence = nullptr); // submit only on main thread
	private:
		vk::Device m_device;
		vk::Queue m_graphicsQueue;
		uint32_t m_graphicsQueueFamilyIndex;
	};

	class RfctSwapChain {
	public:
		vk::SwapchainKHR GetSwapChain() { return m_swapChain.get(); }
		vk::SurfaceFormatKHR GetSurfaceFormat() { return m_surfaceFormat; }
		vk::Extent2D GetExtent() { return m_swapChainExtent; }
	public:
		RfctSwapChain(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface);
		void CreateSwapChain(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface);
		void RecreateSwapChain(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface);
		uint32_t AcquireNextImage(const vk::Semaphore& semaphore, vk::Fence fence, vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface); // will recreate swapchain if unoptimal
	public:
		bool framebufferResized = false;
	private:
		vk::UniqueSwapchainKHR m_swapChain;
		vk::Extent2D m_swapChainExtent;
		vk::SurfaceFormatKHR m_surfaceFormat;
	};

	// surface works a little differently on android
	class RfctSurfaceWrapper {
	public:
		vk::SurfaceKHR GetSurface() { return m_surface; }
	public:
		RfctSurfaceWrapper(vk::SurfaceKHR surfaceArg);
		void NewSurface(vk::Instance instance, vk::SurfaceKHR surfaceArg);
		void DestroySurface(vk::Instance instance);
	private:
		vk::SurfaceKHR m_surface;
	};

	// vma is static on windows, dynamic on android (requires different setups).
	class RfctVulkanMemAllocatorWrapper {
	public:
		VmaAllocator& GetAllocator() { return m_allocator; }
	public:
		RfctVulkanMemAllocatorWrapper(vk::PhysicalDevice physicalDevice, vk::Device device, vk::Instance instance);
		~RfctVulkanMemAllocatorWrapper();
	private:
		VmaAllocator m_allocator;
	};
}
