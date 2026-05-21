#pragma once
#include <glm/glm.hpp>
#include "renderer_p/buffer/vulkan_buffer.h"

namespace rfct {
	struct uboData {
		glm::mat4 vp;
		float globalTime;
	};
	class ubo {
	public:
		static vk::DescriptorSetLayout getDescriptorSetLayout();
		static void destroyDescriptorSetLayout();
	public:
		ubo();
		~ubo();
		void updateUboData(const glm::mat4& vp, float globalTime, float changeSceneEffectMultiplier);
		inline vk::Buffer getBuffer() { return m_buffer.buffer; }
	private:
		uboData m_data;
		VulkanBuffer m_buffer;
		void* m_mappedBuffer;
	};
}