#pragma once
#include "renderer_p\buffer\vulkan_buffer.h"
#include "glm\glm.hpp"
namespace rfct {
	struct uniformBufferObject {
		glm::mat4 vp;
	};
	class cameraUbo {
	public:
		cameraUbo();
		~cameraUbo();
		void updateViewProj(glm::mat4 vp);
	public:
		inline vk::Buffer getBuffer() { return m_buffer.buffer; }
	private:
		uniformBufferObject ubo;
		VulkanBuffer m_buffer;
		void* m_mappedData;

	private:
		static vk::DescriptorSetLayout m_descriptorSetLayout;
	public:
		static vk::DescriptorSetLayout getDescriptorSetLayout();
		static void destroyDescriptorSetLayout();
	};
}