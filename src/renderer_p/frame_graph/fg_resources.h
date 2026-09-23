#pragma once
#include "renderer_p/components/render_objects.h"

namespace rfct {
	struct RfctFrameContext;
	class RfctSwapChain;

	class RfctFrameGraphPerFrameResources {
	public:
		RfctUniformBuffer& GetSceneUniformBuffer() { return m_sceneUniform; }
		RfctUniformBuffer& GetUIUniformBuffer() { return m_UIUniform; }
	public:
		void CreateUniformBuffers(RfctVulkanMemAllocator& memAllocatorWrapper, vk::Device device);
		void CreateSynchronizationStructures(RfctVulkanMemAllocator& allocatorWrapper, RfctQueue& queue, vk::Device device);
		void DestroyUniformBuffers();
	private:
		RfctUniformBuffer m_sceneUniform;
		RfctUniformBuffer m_UIUniform;
	public:
		void WaitImageRenderFinished(vk::Device device);
		void ResetFences(vk::Device device);
		vk::SubmitInfo GetSubmitInfo() const;
	public:
		vk::CommandPool m_commandPool;
		vk::CommandBuffer m_commandBuffer;
		vk::Semaphore m_ImageAvaibleSemaphore;
		vk::Fence m_thisFrameRenderFinishedFence;
	};

	// Made in separate file and struct because the other takes care of the logic of building and executing fg,
	// whilist this one takes care of the lifetime problem
	class RfctFrameGraphResources {
	public:
		RfctFrameGraphPerFrameResources& GetFrameResources(uint32_t i) { return m_perFrameResources[i]; }
	public:
		void PreFrame(const RfctFrameContext& ctx, float changeSceneEffectMultiplier);
		void CreateResources(RfctDevice& deviceWrapper, RfctVulkanInstance& instanceWrapper, RfctQueue& queueWrapper,
			RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper);
		void DestroyResources(RfctVulkanMemAllocator& allocatorWrapper, vk::Device& device);
	private:
		void CreateUniformBuffers(RfctVulkanMemAllocator& memAllocatorWrapper, vk::Device device);
		void CreateRenderPasses(vk::Device device);
		void CreateImages(rfct::RfctDevice& deviceWrapper, RfctVulkanInstance& instanceWrapper, RfctQueue& queueWrapper,
			RfctVulkanMemAllocator& allocatorWrapper, RfctSwapChain& swapChainWrapper);
		void CreateFrameBuffers(RfctSwapChain& swapChainWrapper, vk::Device device);
		void DestroyImages(RfctVulkanMemAllocator& allocatorWrapper, vk::Device& device);
	private:
		RfctRenderPass m_UIRenderPass;
		RfctRenderPass m_presentToColorAttachment;
		RfctRenderPass m_IntermediateClearRenderPass;
		RfctRenderPass m_IntermediateRenderPass;
		RfctRenderPass m_sceneRenderPass;
		std::vector<RfctRenderImage> m_sceneImages;
		std::vector<RfctRenderImage> m_bloom1Images;
		std::vector<RfctRenderImage> m_bloom2Images;
		std::vector<RfctRenderImage> m_swapchainImages;
		std::vector<RfctRenderImage> m_msaaColorImages;
		RfctFrameGraphPerFrameResources m_perFrameResources[RFCT_FRAMES_IN_FLIGHT];
	};
}