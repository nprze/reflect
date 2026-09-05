#pragma once

namespace rfct {
	class renderData;
	class RfctSwapChain;
	class frameData;
	class RfctShader;
	class vulkanRasterizerPipeline
	{
	public:
		vk::Pipeline GetPipeline() { return m_graphicsPipeline.get(); }
	public:
		vulkanRasterizerPipeline(vk::RenderPass renderPass, vk::Device device);
		void CreatePipeline(vk::RenderPass renderPass, vk::Device device);
		void RecordCommandBuffer(frameContext* ctx, RfctSwapChain& swapChainWrapper, frameData& frameData, vk::Framebuffer framebuffer, vk::RenderPass renderPass);
	private:
		RfctShader* m_vertexShader;
		RfctShader* m_fragShader;
		vk::UniquePipelineLayout m_pipelineLayout;
		vk::UniquePipeline m_graphicsPipeline;
	};
}