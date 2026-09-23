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
		void CreateBloomPassResources(RfctRenderPass* renderPass, RfctPipelineManager& pipelineManager, vk::Device device);
		void updateDescSets(RfctRenderImagesManager& imageManager, vk::Device device);
		void blum(RfctFrameContext* ctx, RfctRenderImagesManager& imageManager, RfctSwapChain& swapChain,
			frameSyncDataTemp& fd, vk::RenderPass renderPass, uint32_t imageIndex);
		void recordCommandBuffer(RfctRenderImagesManager& imageManager, RfctSwapChain& swapChain,
			vk::CommandBuffer commandBuffer, vk::RenderPass renderPass, uint32_t imageIndex, uint32_t swapchainImage);
		void onSwapchainExtentChanged(RfctRenderImagesManager& imageManager, vk::Device device);
	private:
		vk::UniqueSampler m_imageSampler;
		RfctRenderPipeline* m_gaussianPipeline;
		RfctRenderPipeline* m_compositePipeline;
		vk::UniqueDescriptorPool m_descriptorPool;
		vk::DescriptorSetLayout m_gaussianPipelineDescLayout;
		std::vector<vk::UniqueDescriptorSet> m_gaussian1SceneImageDescriptorSet; // image 0
		std::vector<vk::UniqueDescriptorSet> m_gaussian2SceneImageDescriptorSet; // image 2
		std::vector<vk::UniqueDescriptorSet> m_compositeImageDescriptorSet; // image 0 and 1
	};
}