#pragma once
#include "renderer_p/shader/vulkan_shader.h"
#include "context.h"
namespace rfct {
	class sceneRenderData;
	class frameData;
	class vulkanRasterizerPipeline
	{
	public:
		vulkanRasterizerPipeline(vk::RenderPass renderPass);
		~vulkanRasterizerPipeline();
		void createPipeline(vk::RenderPass renderPass);
		void recordCommandBuffer(frameContext* ctx, frameData& frameData, vk::Framebuffer framebuffer, vk::RenderPass renderPass);
	private:
		vulkanShader m_vertexShader;
		vulkanShader m_fragShader;
		vk::UniquePipelineLayout m_pipelineLayout;
		vk::UniquePipeline m_graphicsPipeline;
	};
}