#pragma once
#include <vma/vk_mem_alloc.h>

namespace rfct {
	class RfctDevice;
	class RfctQueue;
	class RfctSwapChain;
	class RfctVulkanInstance;
	class RfctVulkanMemAllocator;

	class RfctRenderImage {
	public:
		struct RfctRenderImageSpec {
			// settings
			vk::Extent2D extent = { 1, 1 };
			vk::Format dafaultFormat = vk::Format::eB8G8R8A8Unorm;
			std::string debugName = "renderImage";
			// image create
			VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
			vk::SampleCountFlags imageSamples = vk::SampleCountFlagBits::e1;
			vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
			// runtime settings
			vk::ImageLayout dafaultLayout = vk::ImageLayout::eColorAttachmentOptimal;
		};
	public:
		void TransformLayoutSync(vk::ImageLayout newLayout, RfctDevice& deviceWrapper, RfctQueue& queue);
		void CreateImageAndView(const RfctRenderImageSpec& spec, RfctDevice& deviceWrapper,
			RfctVulkanInstance& instanceWrapper, RfctQueue& queueWrapper, RfctVulkanMemAllocator& allocatorWrapper);
		void InitFrameBuffer(std::vector<RfctRenderImage*> attachments, vk::RenderPass renderPass, vk::Device device);
		void Cleanup(RfctVulkanMemAllocator& allocatorWrapper, vk::Device device);
	private:
		void AllocateImage(const RfctRenderImage::RfctRenderImageSpec& spec, RfctDevice& deviceWrapper,
			RfctQueue& queueWrapper, RfctVulkanMemAllocator& allocatorWrapper);
		void CreateImageView(vk::Device device);
	public:
		bool hasFrameBuffer;
		bool wasAllocatedUsingVMA = false; // usually yes, swap chain could be one exception
		vk::Extent2D m_extent;
		std::string m_debugName;
		vk::Image m_image;
		VmaAllocation m_imageAllocation;
		vk::UniqueImageView m_imageView;
		vk::UniqueFramebuffer m_frameBuffer;
		vk::ImageLayout m_currentLayout = vk::ImageLayout::eUndefined;
		vk::Format m_format;
	};

	class RfctRenderPass {
	public:
		struct RfctRenderPassSpec {
		};
	};

	// temporary solution- want to have framegraph owning render images and frame buffers
	class RfctRenderImagesManager {
	public:
		RfctRenderImagesManager(rfct::RfctDevice& deviceWrapper, rfct::RfctQueue& queueWrapper,
			RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper);
		~RfctRenderImagesManager();
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


		vk::UniqueRenderPass m_UIRenderPass;
		vk::UniqueRenderPass m_presentToColorAttachment;
		vk::UniqueRenderPass m_IntermediateClearRenderPass;
		vk::UniqueRenderPass m_IntermediateRenderPass;
		vk::UniqueRenderPass m_sceneRenderPass;

		std::vector<RfctRenderImage> m_sceneImages;
		std::vector<RfctRenderImage> m_bloom1Images;
		std::vector<RfctRenderImage> m_bloom2Images;
		std::vector<RfctRenderImage> m_swapchainImages;
		std::vector<RfctRenderImage> m_msaaColorImages;
	};
}