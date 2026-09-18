#pragma once
#include <vulkan/vulkan.hpp>

namespace rfct {
	class RfctPipelineManager;
	class RfctRenderPipeline;
	class RfctSwapChain;
	class RfctFrameSyncData;
	struct frameContext;
	class RfctScenePass {
	public:
		void CreatePassResources(vk::RenderPass renderPass, RfctPipelineManager& pipelineManager, vk::Device device);
		void RecordCommandBuffer(frameContext* ctx, RfctSwapChain& swapChainWrapper, RfctFrameSyncData& RfctFrameSyncData, vk::Framebuffer framebuffer, vk::RenderPass renderPass);
	private:
		RfctRenderPipeline* m_pipelineRef;
	};
}