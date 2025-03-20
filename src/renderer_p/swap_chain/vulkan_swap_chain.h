#pragma once
#include <vulkan/vulkan.hpp>
namespace rfct {
	class vulkanSwapChain
	{
	public:
		vulkanSwapChain(vk::Device device, vk::SurfaceKHR surface, vk::PhysicalDevice physicalDevice, vk::Extent2D windowExtent);
		~vulkanSwapChain();
		void createSwapChain();
		void recreateSwapChain();
		void createImageViews();
		void createFrameBuffers();
        void newSurfaceSet(const vk::SurfaceKHR& surArg);
		uint32_t acquireNextImage(const vk::Semaphore& semaphore, vk::Fence fence);
		vk::SwapchainKHR getSwapChain() { return m_swapChain.get(); }
		vk::SurfaceFormatKHR getSurfaceFormat() { return m_surfaceFormat; }
		vk::Image getImage(uint32_t index) { return m_swapChainImage[index]; }
		vk::Framebuffer getFrameBuffer(uint32_t index) { return m_frameBuffers[index].get(); }
		vk::Extent2D getExtent() { return m_swapChainExtent; }
		vk::SurfaceKHR getSurface() { return m_surface; }
		bool framebufferResized = false;
	private:
		vk::Device m_device;
		vk::SurfaceKHR m_surface;
		vk::PhysicalDevice m_physicalDevice;
		vk::UniqueSwapchainKHR m_swapChain;
		std::vector<vk::Image> m_swapChainImage;
		std::vector<vk::UniqueImageView> m_swapChainImageViews;
		std::vector<vk::UniqueFramebuffer> m_frameBuffers;
		vk::Extent2D m_windowExtent;
		vk::Extent2D m_swapChainExtent;
		vk::SurfaceFormatKHR m_surfaceFormat;
	};
} // namespace rfct