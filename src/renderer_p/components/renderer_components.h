#pragma once
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>
#include "platform_window.h"

namespace rfct {
	class RfctShader;
	class frameData;

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

	class RfctQueue {
	public:
		vk::Queue GetPresentQueue() { return m_graphicsQueue; }
		uint32_t GetGraphicsQueueFamilyIndex() { return m_graphicsQueueFamilyIndex; }
	public:
		RfctQueue(vk::Device device, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);
		void SubmitGraphics(const vk::SubmitInfo& submitInfo, vk::Fence fence = nullptr); // submit only on main thread
	private:
		vk::Device m_device;
		vk::Queue m_graphicsQueue;
		uint32_t m_graphicsQueueFamilyIndex;
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

	class RfctSwapChain {
	public:
		struct RfctAcquireNextImageResult {
			bool Succeeded() { return internalResult == vk::Result::eSuccess; }
			bool needsRecreation : 1 = false;
			bool suboptimal : 1 = false;
			vk::Result internalResult;
			uint32_t imageIndex;
		};
		vk::SwapchainKHR GetSwapChain() { return m_swapChain.get(); }
		vk::SurfaceFormatKHR GetSurfaceFormat() { return m_surfaceFormat; }
		vk::Extent2D GetExtent() { return m_swapChainExtent; }
	public:
		RfctSwapChain(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface);
		void CreateSwapChain(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface);
		void RecreateSwapChain(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface);
		RfctAcquireNextImageResult AcquireNextImage(const vk::Semaphore& semaphore, vk::Fence fence, vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface); // will recreate swapchain if unoptimal
	public:
		bool m_framebufferResized = false;
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
	class RfctVulkanMemAllocator {
	public:
		VmaAllocator& GetAllocator() { return m_allocator; }
	public:
		RfctVulkanMemAllocator(vk::PhysicalDevice physicalDevice, vk::Device device, vk::Instance instance);
		~RfctVulkanMemAllocator();
	private:
		VmaAllocator m_allocator;
	};

	class RfctRenderPipeline {
		struct RfctRenderPipelineSpec {
			std::string vertexShaderPath;
			std::string fragmentShaderPath;
			vk::VertexInputBindingDescription vertexInputBindingDescription;
			std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;
			std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
			bool MSAA4x = false;
		};
	public:
		RfctRenderPipeline(const RfctRenderPipelineSpec& spec, vk::RenderPass renderPass, vk::Device device);
		void CreatePipeline(const RfctRenderPipelineSpec& spec, vk::RenderPass renderPass, vk::Device device);
		void RecordCommandBuffer(frameContext* ctx, RfctSwapChain& swapChainWrapper, frameData& frameData, vk::Framebuffer framebuffer, vk::RenderPass renderPass);
	private:
		RfctShader* m_vertexShader;
		RfctShader* m_fragShader;
		vk::UniquePipelineLayout m_pipelineLayout;
		vk::UniquePipeline m_graphicsPipeline;
	};
}
