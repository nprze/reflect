#pragma once
#include "renderer_p/shader/vulkan_shader.h"

namespace rfct {
	class renderData;
	class frameData;
	class vulkanRasterizerPipeline
	{
	public:
		vulkanRasterizerPipeline(vk::RenderPass renderPass, vk::Device device);
		void CreatePipeline(vk::RenderPass renderPass, vk::Device device);
		void RecordCommandBuffer(frameContext* ctx, RfctSwapChain& swapChainWrapper, frameData& frameData, vk::Framebuffer framebuffer, vk::RenderPass renderPass);
	private:
		vulkanShader m_vertexShader;
		vulkanShader m_fragShader;
		vk::UniquePipelineLayout m_pipelineLayout;
		vk::UniquePipeline m_graphicsPipeline;
	};
}