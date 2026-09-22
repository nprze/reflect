#pragma once
#include <vulkan/vulkan.hpp>
#include "renderer_p/components/render_objects.h"

namespace rfct {
	class RfctPipelineManager;
	class RfctRenderPipeline;
	class RfctSwapChain;
	class frameSyncDataTemp;
	struct frameContext;

	class RfctSceneGraphicsPass {
	public:
		void CreatePassResources(RfctRenderPass* renderPass, RfctPipelineManager& pipelineManager, vk::Device device);
		void DestroyPassResources(vk::Device device);
		void RecordCommandBuffer(frameContext* ctx, RfctSwapChain& swapChainWrapper, frameSyncDataTemp& frameSyncDataTemp, vk::Framebuffer framebuffer);
	private:
		RfctRenderPipeline* m_pipelineRef;
		RfctRenderPass* m_renderPassRef;
	};
}