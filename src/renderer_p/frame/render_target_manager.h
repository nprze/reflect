#pragma once
#include <vma/vk_mem_alloc.h>

namespace rfct {
	class RfctDevice;
	class RfctQueue;
	class RfctSwapChain;
	class RfctVulkanMemAllocator;
	// class designed to hold the framebuffers and images
	class renderImagesManager {
	public:
		renderImagesManager(rfct::RfctDevice& deviceWrapper, rfct::RfctQueue& queueWrapper,
			RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper);
		~renderImagesManager();
	private:
		void CreateResources(rfct::RfctDevice& deviceWrapper, rfct::RfctQueue& queueWrapper,
			RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper);
		void CreateImageViews(RfctSwapChain& swapChainWrapper, vk::Device device);
		void CreateImages(rfct::RfctDevice& deviceWrapper, rfct::RfctQueue& queueWrapper,
			RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper);
		void CreateFrameBuffers(RfctSwapChain& swapChainWrapper, vk::Device device);
		void CreateMSAAres(RfctSwapChain& swapChainWrapper, RfctVulkanMemAllocator& allocatorWrapper, vk::Device device, vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e4);
		void CleanupMSAAres(RfctVulkanMemAllocator& allocatorWrapper);
		void CleanupImages(RfctVulkanMemAllocator& allocatorWrapper);
		void CreateRenderPasses(vk::Device device, vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e4);
	public:
		vk::Image GetSceneImage(uint32_t index) { return m_sceneImages[index]; }
		vk::Image GetBloom1Image(uint32_t index) { return m_bloom1Images[index]; }
		vk::Image GetBloom2Image(uint32_t index) { return m_bloom2Images[index]; }
		vk::Image GetswapchainImage(uint32_t index) { return m_swapchainImages[index]; }
		vk::Framebuffer GetSceneFrameBuffer(uint32_t index) { return m_sceneFramebuffers[index].get(); }
		vk::Framebuffer GetSwapChainFrameBuffer(uint32_t index) {  return m_swapchainFramebuffers[index].get(); }
		vk::Framebuffer GetBloom1FrameBuffer(uint32_t index) { return m_bloom1Framebuffers[index].get(); }
		vk::Framebuffer GetBloom2FrameBuffer(uint32_t index) { return m_bloom2Framebuffers[index].get(); }
		vk::ImageView GetSceneImageView(uint32_t index) { return m_sceneImageViews[index].get(); }
		vk::ImageView GetBloom1ImageView(uint32_t index) { return m_bloom1ImageViews[index].get(); }
		vk::ImageView GetBloom2ImageView(uint32_t index) { return m_bloom2ImageViews[index].get(); }
		vk::RenderPass GetUIRenderPass() { return m_UIRenderPass.get(); }
		vk::RenderPass GetpresentToColorAttachmentRenderPass() { return m_presentToColorAttachment.get(); }
		vk::RenderPass GetIntermediateClearRenderPass() { return m_IntermediateClearRenderPass.get(); }
		vk::RenderPass GetIntermediateRenderPass() { return m_IntermediateRenderPass.get(); }
		vk::RenderPass GetSceneRenderPass() { return m_sceneRenderPass.get(); }
	private:
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