#pragma once
#include "renderer_p\shader\vulkan_shader.h"
#include "renderer_p\frame\frame_data.h"
#include "context.h"
namespace rfct {
	class sceneRenderData;
	class vulkanRasterizerPipeline
	{
	public:
		vulkanRasterizerPipeline();
		~vulkanRasterizerPipeline();
		void createPipeline();
		void createRenderPass();
		void recordCommandBuffer(frameContext* ctx, const sceneRenderData& renderdata, frameData& frameData, vk::Framebuffer framebuffer, uint32_t imageIndex);
		vk::RenderPass getRenderPass() { return m_renderPass.get(); }
	private:
		vulkanShader m_vertexShader;
		vulkanShader m_fragShader;
		vk::UniquePipelineLayout m_pipelineLayout;
		vk::UniquePipeline m_graphicsPipeline;
		vk::UniqueRenderPass m_renderPass;
	};
}