#pragma once
#include <glm/glm.hpp>
#include "renderer_p/shader/vulkan_shader.h"
#include "renderer_p/frame/frame_data.h"

namespace rfct {

	struct layoutTemporaryHolder {
		vk::PipelineLayout pipeline;
		vk::DescriptorSetLayout descSet;
	};

	struct bloomSamplerHolder {
		vk::UniqueSampler m_sampler;
		bloomSamplerHolder();
	};


	struct gaussianPushConstants {
		glm::vec2 dir;
		float res;
	};
	class postprocPipeline {
	public:
		postprocPipeline(vk::RenderPass renderPass, vulkanShader* shaderRef, const std::string& fragmentShaderPath, layoutTemporaryHolder pipelineLayoutStuff); // the will be responsible for cleanup after pipelineLayout and descriptor set descSet
		~postprocPipeline();
	private:
		vulkanShader* m_vertexShader;
		vulkanShader m_fragShader;
		vk::UniquePipeline m_pipeline;
		vk::DescriptorSetLayout m_descSetLayout;
		vk::PipelineLayout m_pipelineLayout;
		friend class bloomResurcesHolder;
	};
	class bloomResurcesHolder {
	public:
		bloomResurcesHolder(vk::RenderPass renderPass);
		void updateDescSets();
		void blum(frameContext* ctx, frameData& fd, vk::RenderPass renderPass, uint32_t imageIndex);
		void recordCommandBuffer(vk::CommandBuffer commandBuffer, vk::RenderPass renderPass, uint32_t imageIndex);
	private:
		vulkanShader vertexShader;
		bloomSamplerHolder m_imageSampler;
		postprocPipeline m_gaussianPipeline;
		postprocPipeline m_compositePipeline;

		vk::UniqueDescriptorPool m_descriptorPool;

		std::vector<vk::UniqueDescriptorSet> m_gaussian1SceneImageDescriptorSet; // image 0
		std::vector<vk::UniqueDescriptorSet> m_gaussian2SceneImageDescriptorSet; // image 2
		std::vector<vk::UniqueDescriptorSet> m_compositeImageDescriptorSet; // image 0 and 1

		// using prebaked command buffers bcs literally nothing changes frame to frame in this  
		vk::UniqueCommandPool m_bloomCommandPool;
		std::vector<vk::UniqueCommandBuffer> m_bloomCommandBuffer;
	};
}