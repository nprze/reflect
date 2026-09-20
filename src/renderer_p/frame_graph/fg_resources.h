#pragma once
#include "renderer_p/components/render_objects.h"

namespace rfct {
	struct frameContext;
	class RfctFrameGraphPerFrameResources {
	public:
		RfctUniformBuffer& GetSceneUniformBuffer() { return m_sceneUniform; }
		RfctUniformBuffer& GetUIUniformBuffer() { return m_UIUniform; }
	public:
		void CreateUniformBuffers(RfctVulkanMemAllocator& memAllocatorWrapper, vk::Device device);
		void DestroyUniformBuffers();
	private:
		RfctUniformBuffer m_sceneUniform;
		RfctUniformBuffer m_UIUniform;
	};

	// Made in separate file and struct because the other takes care of the logic of building and executing fg,
	// whilist this one takes care of the lifetime problem
	class RfctFrameGraphResources {
	public:
		RfctFrameGraphPerFrameResources& GetFrameResources(uint32_t i) { return m_perFrameResources[i]; }
	public:
		void PreFrame(const frameContext& ctx, float changeSceneEffectMultiplier);
		void CreateUniformBuffers(RfctVulkanMemAllocator& memAllocatorWrapper, vk::Device device);
		void DestroyResources();
	private:
		vk::UniqueRenderPass m_UIRenderPass;
		vk::UniqueRenderPass m_presentToColorAttachment;
		vk::UniqueRenderPass m_IntermediateClearRenderPass;
		vk::UniqueRenderPass m_IntermediateRenderPass;
		vk::UniqueRenderPass m_sceneRenderPass;
		RfctFrameGraphPerFrameResources m_perFrameResources[RFCT_FRAMES_IN_FLIGHT];
	};
}