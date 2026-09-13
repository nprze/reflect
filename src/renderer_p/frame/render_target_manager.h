#pragma once
#include <vulkan/vulkan.hpp>
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
			bool allocateImage = true;
			vk::Image image = nullptr; // should hold valid image handle if allocateImage is false
			VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
			vk::SampleCountFlagBits imageSamples = vk::SampleCountFlagBits::e1;
			vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
			// runtime settings
			vk::ImageLayout dafaultLayout = vk::ImageLayout::eColorAttachmentOptimal;
		};
	public:
		void TransformLayoutSync(vk::ImageLayout newLayout, RfctDevice& deviceWrapper, RfctQueue& queue);
		void TransformLayoutAsync(vk::ImageLayout newLayout, vk::CommandBuffer commandBuffer);
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
		vk::SampleCountFlags m_sampleCount;
	};

	// temporary solution- want to have framegraph owning render images and frame buffers
	class RfctRenderImagesManager {
	public:
		RfctRenderImage& GetSceneImage(size_t index) { return m_sceneImages[index]; }
		RfctRenderImage& GetBloom1Image(size_t index) { return m_bloom1Images[index]; }
		RfctRenderImage& GetBloom2Image(size_t index) { return m_bloom2Images[index]; }
		RfctRenderImage& GetSwapChainImage(size_t index) { return m_swapchainImages[index]; }
		vk::RenderPass GetUIRenderPass() { return m_UIRenderPass.get(); }
		vk::RenderPass GetpresentToColorAttachmentRenderPass() { return m_presentToColorAttachment.get(); }
		vk::RenderPass GetIntermediateClearRenderPass() { return m_IntermediateClearRenderPass.get(); }
		vk::RenderPass GetIntermediateRenderPass() { return m_IntermediateRenderPass.get(); }
		vk::RenderPass GetSceneRenderPass() { return m_sceneRenderPass.get(); }
	public:
		RfctRenderImagesManager(rfct::RfctDevice& deviceWrapper, RfctVulkanInstance& instanceWrapper, rfct::RfctQueue& queueWrapper,
			RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper);
		~RfctRenderImagesManager();
		void CreateResources(rfct::RfctDevice& deviceWrapper, RfctVulkanInstance& instanceWrapper, rfct::RfctQueue& queueWrapper,
			RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper);
	private:
		void CreateImages(vk::SampleCountFlagBits msaaSamples, rfct::RfctDevice& deviceWrapper, RfctVulkanInstance& instanceWrapper, 
			rfct::RfctQueue& queueWrapper, RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper);
		void CreateFrameBuffers(RfctSwapChain& swapChainWrapper, vk::Device device);
		void CleanupImages(RfctVulkanMemAllocator& allocatorWrapper, vk::Device& device);
		void CreateRenderPasses(vk::Device device, vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e4);
	private:
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