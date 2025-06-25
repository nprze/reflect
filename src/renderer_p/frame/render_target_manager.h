#pragma once
#include "renderer_p/swap_chain/vulkan_swap_chain.h"

namespace rfct {

	class renderImagesManager {
	public:
		vulkanSwapChain& getSwapChain() { return m_swapChain; }
		// class designed to hold the framebuffers and images
		uint32_t acquireNextImage(const vk::Semaphore& sem, vk::Fence fence);
		renderImagesManager();
		~renderImagesManager();
		void createResources();
		void createImageViews();
		void createFrameBuffers();
		void createMSAAres(vk::SampleCountFlagBits msaaSamples);
		void cleanupMSAAres();
		void getSwapChainImages();
		void createRenderPasses();


		vk::Image getImage(uint32_t index) { return m_swapChainImages[index]; }
		vk::Framebuffer getSceneFrameBuffer(uint32_t index) { return m_frameBuffers[index].get(); }
		vk::Framebuffer getUIFrameBuffer(uint32_t index) { return m_UIframeBuffers[index].get(); }

		vk::RenderPass getUIRenderPass() { return m_UIRenderPass.get(); }
		vk::RenderPass getSceneRenderPass() { return m_sceneRenderPass.get(); }

	private:
		vulkanSwapChain m_swapChain;

		vk::UniqueRenderPass m_UIRenderPass;
		vk::UniqueRenderPass m_sceneRenderPass;

		std::vector<vk::Image> m_swapChainImages;
		std::vector<vk::UniqueImageView> m_swapChainImageViews;
		std::vector<vk::UniqueFramebuffer> m_frameBuffers; // this has 2 attachments bcs rasterizer pipoline uses anti-aliasing
		std::vector<vk::Image> m_msaaColorImages;
		std::vector<vk::UniqueFramebuffer> m_UIframeBuffers; // this has 1 attachment bcs ui (and debugdraw) do not use anti-aliasing
		std::vector<VmaAllocation> m_msaaImageAllocations;
		std::vector<vk::UniqueImageView> m_msaaColorImageViews;
	};
}