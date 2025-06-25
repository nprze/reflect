#pragma once
#include <vma/vk_mem_alloc.h>
namespace rfct {
	class vulkanSwapChain
	{
	public:
		void createSwapChain();
		void recreateSwapChain();

		uint32_t acquireNextImage(const vk::Semaphore& semaphore, vk::Fence fence);
		vk::SwapchainKHR getSwapChain() { return m_swapChain.get(); }
		vk::SurfaceFormatKHR getSurfaceFormat() { return m_surfaceFormat; }
		vk::Extent2D getExtent() { return m_swapChainExtent; }
		bool framebufferResized = false;
	private:
		vk::UniqueSwapchainKHR m_swapChain;
		vk::Extent2D m_swapChainExtent;
		vk::SurfaceFormatKHR m_surfaceFormat;
	private:
		vulkanSwapChain();
		~vulkanSwapChain();
		friend class renderImagesManager;
	};
} // namespace rfct