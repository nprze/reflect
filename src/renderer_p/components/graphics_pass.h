#pragma once
#include <vulkan/vulkan.hpp>
#include "renderer_p/components/render_objects.h"

namespace rfct {
	class RfctPipelineManager;
	class RfctRenderPipeline;
	class RfctSwapChain;
	class frameSyncDataTemp;
	struct RfctFrameContext;

	class RfctSceneGraphicsPass {
	public:
		void CreatePassResources(RfctRenderPass* renderPass, RfctPipelineManager& pipelineManager, vk::Device device);
		void DestroyPassResources(vk::Device device);
		void RecordCommandBuffer(RfctFrameContext& ctx, frameSyncDataTemp& frameSyncDataTemp, vk::Framebuffer framebuffer);
	private:
		RfctRenderPipeline* m_pipelineRef;
		RfctRenderPass* m_renderPassRef;
	};

	class RfctBloomGraphicsPass {
	public:
		struct RfctBloomPushConstants {
			glm::vec2 dir;
			float res;
		};
	public:
		void CreateBloomPassResources(RfctRenderPass* gaussianPass, RfctRenderPass* compositePass, RfctPipelineManager& pipelineManager, vk::Device device);
		void UpdateGaussian1DescSets(uint32_t frameInFlightIndex, vk::ImageView imageView, vk::Device device);
		void UpdateGaussian2DescSets(uint32_t frameInFlightIndex, vk::ImageView imageView, vk::Device device);
		void UpdateCompositeDescSets(uint32_t frameInFlightIndex, vk::ImageView imageView0, vk::ImageView imageView1, vk::Device device);
		void RecordCommandBuffer(RfctFrameContext& ctx);
		void onSwapchainExtentChanged(RfctRenderImagesManager& imageManager, vk::Device device);
	private:
		vk::Sampler m_imageSampler;
		RfctRenderPipeline* m_gaussianPipelineRef;
		RfctRenderPipeline* m_compositePipelineRef;
		RfctRenderPass* m_gaussianPass;
		RfctRenderPass* m_compositePass;
		vk::DescriptorPool m_descriptorPool;
		vk::DescriptorSetLayout m_gaussianPipelineDescLayout;
		vk::DescriptorSetLayout m_compositePipelineDescLayout;
		std::vector<vk::DescriptorSet> m_gaussian1ImageDescriptorSet;
		std::vector<vk::DescriptorSet> m_gaussian2ImageDescriptorSet;
		std::vector<vk::DescriptorSet> m_compositeImageDescriptorSet;
	};
}