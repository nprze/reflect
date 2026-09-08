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

	// A class to hold camera descriptors that are per frame in flight.
	// Camera descriptors are separate, bcs they should always use set 0, binding 0 in shader also camera can freely be reused between scenes so there is no point in moving it to scene specific render data
	// The ssbo (matrices data) is held in scene render data class
	class RfctDescriptor {
	public:
		RfctDescriptor(uint32_t size = 1);
		void bindCameraUbo(vk::Buffer ubo, uint32_t index);
		vk::DescriptorSet& getCameraDescSet(uint32_t index) { return m_cameraUboDescSet[index].get(); }
	private:
		vk::UniqueDescriptorPool m_descriptorPool;
		std::vector<vk::UniqueDescriptorSet> m_cameraUboDescSet;
	};
}