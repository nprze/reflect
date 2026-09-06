#pragma once
#include <vma/vk_mem_alloc.h>

namespace rfct {
	class RfctDevice;
	class RfctQueue;
	class RfctSwapChain;
	class RfctVulkanMemAllocator;

	class RfctRenderImage {
	public:
		struct RfctRenderImageSpec {
			bool initalizeFramebuffer = false;
			vk::Extent2D extent = { 1, 1 };
			vk::Format dafaultFormat = vk::Format::eB8G8R8A8Unorm;
			vk::ImageLayout dafaultLayout = vk::ImageLayout::eColorAttachmentOptimal;
			std::string debugName = "image";
			VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
		};
	public:
		void TransformLayoutSync(vk::ImageLayout newLayout, RfctDevice& deviceWrapper, RfctQueue& queue);
		static void CreateImageNoDeps(RfctRenderImage& renderImageOut, const RfctRenderImage::RfctRenderImageSpec& spec, vk::Device device);
	private:
		void AllocateImage(const RfctRenderImage::RfctRenderImageSpec& spec, RfctDevice& deviceWrapper,
			RfctQueue& queueWrapper, RfctVulkanMemAllocator& allocatorWrapper);
		void CreateImageView(RfctSwapChain& swapChainWrapper, vk::Device device);
	public:
		bool hasFrameBuffer;
		bool wasAllocatedUsingVMA; // usually yes, swap chain could be one exception
		vk::Extent2D m_extent;
		std::string m_debugName;
		vk::Image m_image;
		VmaAllocation m_imageAllocation;
		vk::UniqueImageView m_imageView;
		vk::UniqueFramebuffer m_frameBuffer;
		vk::ImageLayout m_currentLayout = vk::ImageLayout::eUndefined;
		vk::Format m_format;
	};

	// class designed to hold the framebuffers and images
	class renderImagesManager {
	public:
		renderImagesManager(rfct::RfctDevice& deviceWrapper, rfct::RfctQueue& queueWrapper,
			RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper);
		~renderImagesManager();
		void CreateResources(rfct::RfctDevice& deviceWrapper, rfct::RfctQueue& queueWrapper,
			RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper);
	private:
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