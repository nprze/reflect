#pragma once
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>
#include "renderer_p/components/render_objects.h"

namespace rfct {
	class RfctDevice;
	class RfctQueue;
	class RfctSwapChain;
	class RfctVulkanInstance;
	class RfctVulkanMemAllocator;

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
		void CreateResources(rfct::RfctDevice& deviceWrapper, RfctVulkanInstance& instanceWrapper, rfct::RfctQueue& queueWrapper,
			RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper);
		void CleanupResources(RfctVulkanMemAllocator& allocatorWrapper, vk::Device& device);
	private:
		void CreateImages(vk::SampleCountFlagBits msaaSamples, rfct::RfctDevice& deviceWrapper, RfctVulkanInstance& instanceWrapper, 
			rfct::RfctQueue& queueWrapper, RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper);
		void CreateFrameBuffers(RfctSwapChain& swapChainWrapper, vk::Device device);
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