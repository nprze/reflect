#pragma once
#include <vulkan/vulkan.hpp>
#include <string>

namespace rfct {
    class RfctShader {
    public:
        RfctShader(vk::Device device, const std::string& spirvFilePath);
        inline vk::ShaderModule getShaderModule() { return m_shaderModule.get(); }
    private:
        vk::UniqueShaderModule m_shaderModule;
    };

	class RfctRenderPipeline {
		struct RfctRenderPipelineSpec {
			std::string vertexShaderPath;
			std::string fragmentShaderPath;
			vk::VertexInputBindingDescription vertexInputBindingDescription;
			std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;
			std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
			bool MSAA4x = false;
		};
	public:
		RfctRenderPipeline(const RfctRenderPipelineSpec& spec, vk::RenderPass renderPass, vk::Device device);
		void CreatePipeline(const RfctRenderPipelineSpec& spec, vk::RenderPass renderPass, vk::Device device);
	private:
		RfctShader* m_vertexShader;
		RfctShader* m_fragShader;
		vk::UniquePipelineLayout m_pipelineLayout;
		vk::UniquePipeline m_graphicsPipeline;
	};
}