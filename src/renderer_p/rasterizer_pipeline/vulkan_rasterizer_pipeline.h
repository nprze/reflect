#pragma once
#include "renderer_p/shader/vulkan_shader.h"
#include "context.h"
namespace rfct {
	class sceneRenderData;
	class frameData;
	class vulkanRasterizerPipeline
	{
	public:
		vulkanRasterizerPipeline();
		~vulkanRasterizerPipeline();
		void createPipeline();
		void createRenderPass();
		void recordCommandBuffer(frameContext* ctx, frameData& frameData, vk::Framebuffer framebuffer);
		vk::RenderPass getRenderPass() { return m_renderPass.get(); }
	private:
		vulkanShader m_vertexShader;
		vulkanShader m_fragShader;
		vk::UniquePipelineLayout m_pipelineLayout;
		vk::UniquePipeline m_graphicsPipeline;
		vk::UniqueRenderPass m_renderPass;
	};
}