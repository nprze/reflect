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
	private:
		void createResources();
		void createImageViews();
		void createImages();
		void createFrameBuffers();
		void createMSAAres(vk::SampleCountFlagBits msaaSamples);
		void cleanupMSAAres();
		void cleanupImages();
		void getSwapChainImages();
		void createRenderPasses();

	public:

		vk::Image getSceneImage(uint32_t index) { return m_sceneImages[index]; }
		vk::Image getBloom1Image(uint32_t index) { return m_bloom1Images[index]; }
		vk::Image getBloom2Image(uint32_t index) { return m_bloom2Images[index]; }
		vk::Image getswapchainImage(uint32_t index) { return m_swapchainImages[index]; }

		vk::Framebuffer getSceneFrameBuffer(uint32_t index) { return m_sceneFramebuffers[index].get(); }
		vk::Framebuffer getSwapChainFrameBuffer(uint32_t index) {  return m_swapchainFramebuffers[index].get(); }
		vk::Framebuffer getBloom1FrameBuffer(uint32_t index) { return m_bloom1Framebuffers[index].get(); }
		vk::Framebuffer getBloom2FrameBuffer(uint32_t index) { return m_bloom2Framebuffers[index].get(); }

		vk::ImageView getSceneImageView(uint32_t index) { return m_sceneImageViews[index].get(); }
		vk::ImageView getBloom1ImageView(uint32_t index) { return m_bloom1ImageViews[index].get(); }
		vk::ImageView getBloom2ImageView(uint32_t index) { return m_bloom2ImageViews[index].get(); }

		vk::RenderPass getUIRenderPass() { return m_UIRenderPass.get(); }
		vk::RenderPass getpresentToColorAttachmentRenderPass() { return m_presentToColorAttachment.get(); }
		vk::RenderPass getIntermediateClearRenderPass() { return m_IntermediateClearRenderPass.get(); }
		vk::RenderPass getIntermediateRenderPass() { return m_IntermediateRenderPass.get(); }
		vk::RenderPass getSceneRenderPass() { return m_sceneRenderPass.get(); }
	private:
		vulkanSwapChain m_swapChain;

		vk::UniqueRenderPass m_UIRenderPass;
		vk::UniqueRenderPass m_presentToColorAttachment;
		vk::UniqueRenderPass m_IntermediateClearRenderPass;
		vk::UniqueRenderPass m_IntermediateRenderPass;
		vk::UniqueRenderPass m_sceneRenderPass;

		// image 0
		std::vector<vk::Image> m_sceneImages;
		std::vector<vk::UniqueImageView> m_sceneImageViews;
		std::vector<vk::UniqueFramebuffer> m_sceneFramebuffers; // this has 2 attachments bcs rasterizer pipoline uses anti-aliasing
		std::vector<VmaAllocation> m_sceneImagesAllocations;
		
		// image 1
		std::vector<vk::Image> m_bloom1Images;
		std::vector<vk::UniqueImageView> m_bloom1ImageViews;
		std::vector<vk::UniqueFramebuffer> m_bloom1Framebuffers; // 1 attachment
		std::vector<VmaAllocation> m_bloom1ImagesAllocations;

		// image 2
		std::vector<vk::Image> m_bloom2Images;
		std::vector<vk::UniqueImageView> m_bloom2ImageViews;
		std::vector<vk::UniqueFramebuffer> m_bloom2Framebuffers; // 1 attachment
		std::vector<VmaAllocation> m_bloom2ImagesAllocations;


		// image 3
		std::vector<vk::Image> m_swapchainImages;
		std::vector<vk::UniqueImageView> m_swapChainImageViews; // 1 attachment
		std::vector<vk::UniqueFramebuffer> m_swapchainFramebuffers;



		// resouces for image 0, for antialiasing
		std::vector<vk::Image> m_msaaColorImages;
		std::vector<vk::UniqueImageView> m_msaaColorImageViews;
		std::vector<VmaAllocation> m_msaaImageAllocations;

	};
}