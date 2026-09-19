#pragma once
#include "renderer_p/components/render_objects.h"

namespace rfct {
	struct frameContext;
	// Made in separate file and struct because the other takes care of the logic of building and executing fg,
	// whilist this one takes care of the lifetime problem
	class RfctFrameGraphResources {
	public:
		vk::DescriptorSet& GetCameraUboDescSet() { return m_sceneUniform.GetCameraDescSet(); }
		vk::DescriptorSet& GetUICameraUboDescSet() { return m_UIUniform.GetCameraDescSet(); }
	public:
		void CreateUniformBuffers(RfctVulkanMemAllocator& memAllocatorWrapper, vk::Device device);
		void PreFrame(const frameContext& ctx, float changeSceneEffectMultiplier);
	private:
		RfctUniformBuffer m_sceneUniform;
		RfctUniformBuffer m_UIUniform;
	};
}