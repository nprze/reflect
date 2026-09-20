#pragma once
#include <vulkan/vulkan.hpp>

namespace rfct {
	class RfctPipelineManager;
	class RfctRenderPipeline;
	class RfctSwapChain;
	class frameSyncDataTemp;
	struct frameContext;

	class RfctScenePass {
	public:
		void CreatePassResources(vk::RenderPass renderPass, RfctPipelineManager& pipelineManager, vk::Device device);
		void DestroyPassResources(vk::Device device);
		void RecordCommandBuffer(frameContext* ctx, RfctSwapChain& swapChainWrapper, frameSyncDataTemp& frameSyncDataTemp, vk::Framebuffer framebuffer, vk::RenderPass renderPass);
	private:
		RfctRenderPipeline* m_pipelineRef;
	};
}