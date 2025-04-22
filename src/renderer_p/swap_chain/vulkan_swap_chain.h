#pragma once
#include <vma/vk_mem_alloc.h>
namespace rfct {
	class vulkanSwapChain
	{
	public:
		void createSwapChain();
		void recreateSwapChain();
		void createImageViews();
		void createFrameBuffers();
		void createMSAAres(vk::SampleCountFlagBits msaaSamples);
		void cleanupMSAAres();

		uint32_t acquireNextImage(const vk::Semaphore& semaphore, vk::Fence fence);
		vk::SwapchainKHR getSwapChain() { return m_swapChain.get(); }
		vk::SurfaceFormatKHR getSurfaceFormat() { return m_surfaceFormat; }
		vk::Image getImage(uint32_t index) { return m_swapChainImage[index]; }
		vk::Framebuffer getSceneFrameBuffer(uint32_t index) { return m_frameBuffers[index].get(); }
		vk::Framebuffer getUIFrameBuffer(uint32_t index) { return m_UIframeBuffers[index].get(); }
		vk::Extent2D getExtent() { return m_swapChainExtent; }
		bool framebufferResized = false;
	private:
		vk::Device m_device;
		vk::PhysicalDevice m_physicalDevice;
		vk::UniqueSwapchainKHR m_swapChain;
		std::vector<vk::Image> m_swapChainImage;
		std::vector<vk::UniqueImageView> m_swapChainImageViews;
		std::vector<vk::UniqueFramebuffer> m_frameBuffers; // this has 2 attachments bcs rasterizer pipoline uses anti-aliasing
		std::vector<vk::Image> m_msaaColorImages;
		std::vector<vk::UniqueFramebuffer> m_UIframeBuffers; // this has 1 attachment bcs ui (and debugdraw) do not use anti-aliasing
		std::vector<VmaAllocation> m_msaaImageAllocations;
		std::vector<vk::UniqueImageView> m_msaaColorImageViews;
		vk::Extent2D m_windowExtent;
		vk::Extent2D m_swapChainExtent;
		vk::SurfaceFormatKHR m_surfaceFormat;
	private:
		vulkanSwapChain(vk::Device device, vk::PhysicalDevice physicalDevice, vk::Extent2D windowExtent);
		~vulkanSwapChain();
		friend class vulkanDevice;
	};
} // namespace rfct