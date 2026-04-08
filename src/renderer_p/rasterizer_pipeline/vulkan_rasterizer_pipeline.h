#pragma once
#include "context.h"
#include "renderer_p/shader/vulkan_shader.h"

namespace rfct {
	class renderData;
	class frameData;
	class vulkanRasterizerPipeline
	{
	public:
		vulkanRasterizerPipeline(vk::RenderPass renderPass);
		void createPipeline(vk::RenderPass renderPass);
		void recordCommandBuffer(frameContext* ctx, frameData& frameData, vk::Framebuffer framebuffer, vk::RenderPass renderPass);
	private:
		vulkanShader m_vertexShader;
		vulkanShader m_fragShader;
		vk::UniquePipelineLayout m_pipelineLayout;
		vk::UniquePipeline m_graphicsPipeline;
	};
}