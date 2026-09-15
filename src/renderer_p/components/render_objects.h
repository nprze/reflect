#pragma once
#include <vulkan/vulkan.hpp>
#include <string>
#include "renderer_p/buffer/vulkan_buffer.h"
#include <glm/glm.hpp>

namespace rfct {
    class RfctShader {
    public:
        vk::ShaderModule getShaderModule() { return m_shaderModule.get(); }
    public:
        RfctShader(vk::Device device, const std::string& spirvFilePath);
    private:
        vk::UniqueShaderModule m_shaderModule;
    };

	class RfctRenderPipeline {
	public:
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

	struct RfctUniformData {
		glm::mat4 vp;
		float globalTime;
		float changeSceneEffectMultiplier;
	};
	class RfctUniformBuffer {
	public:
		static vk::DescriptorSetLayout GetUniformDescriptorSetLayout(vk::Device device);
		static void DestroyUniformDescriptorSetLayout(vk::Device device);
	public:
		vk::DescriptorSet& GetCameraDescSet() { return m_cameraUboDescSet.get(); }
		vk::Buffer GetBuffer() { return m_buffer.buffer; }
	public:
		RfctUniformBuffer(vk::Device device);
		void DestroyUniformBuffer();
		void UpdateUniformData(const RfctUniformData& newData);
	private:
		void BindBufferToDescriptor(vk::Buffer buffer, vk::Device device);
	private:
		VulkanBuffer m_buffer;
		void* m_mappedBuffer;
		vk::UniqueDescriptorPool m_descriptorPool;
		vk::UniqueDescriptorSet m_cameraUboDescSet;
	};
}