#pragma once
#include <glm/glm.hpp>
#include "renderer_p/frame/frame_data.h"

namespace rfct {
	class RfctShader;
	class renderImagesManager;
	class RfctSwapChain;
	struct layoutTemporaryHolder {
		vk::PipelineLayout pipeline;
		vk::DescriptorSetLayout descSet;
	};
	struct bloomSamplerHolder {
		vk::UniqueSampler m_sampler;
		bloomSamplerHolder(vk::Device device);
	};
	struct gaussianPushConstants {
		glm::vec2 dir;
		float res;
	};
	class postprocPipeline {
	public:
		postprocPipeline(vk::Device device, vk::RenderPass renderPass, const std::string& vertexShaderPath,
			const std::string& fragmentShaderPath, layoutTemporaryHolder pipelineLayoutStuff);
		~postprocPipeline();
	private:
		RfctShader* m_vertexShader;
		RfctShader* m_fragShader;
		vk::UniquePipeline m_pipeline;
		vk::DescriptorSetLayout m_descSetLayout;
		vk::PipelineLayout m_pipelineLayout;

		friend class bloomResurcesHolder;
	};
	class bloomResurcesHolder {
	public:
		bloomResurcesHolder(RfctQueue& queue, renderImagesManager& imageManager, vk::RenderPass renderPass, vk::Device device);
		void updateDescSets(renderImagesManager& imageManager, vk::Device device);
		void blum(frameContext* ctx, renderImagesManager& imageManager, RfctSwapChain& swapChain,
			frameData& fd, vk::RenderPass renderPass, uint32_t imageIndex);
		void recordCommandBuffer(renderImagesManager& imageManager, RfctSwapChain& swapChain,
			vk::CommandBuffer commandBuffer, vk::RenderPass renderPass, uint32_t imageIndex, uint32_t swapchainImage);
		void onSwapchainExtentChanged(renderImagesManager& imageManager, vk::Device device);
	private:
		RfctShader* vertexShader;
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