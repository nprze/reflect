#pragma once
#include "renderer_p\shader\vulkan_shader.h"
#include "renderer_p\frame\frame_data.h"
#include "font\font.h"
namespace rfct {
	struct glyphsRenderData {
		inline glyphsRenderData(uint32_t size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage) :buffer(size, usage, memoryUsage), bufferOffset(0), vertexCount(0) {
 			bufferMappedMemory = buffer.Map();
		};
		inline ~glyphsRenderData() { buffer.Unmap(); };
		inline void postFrame() { bufferOffset = 0; vertexCount = 0; };
		VulkanBuffer buffer;
		uint32_t bufferOffset;
		uint32_t vertexCount;
		void* bufferMappedMemory;
		
	};
	class UIPipeline {
	public:
		UIPipeline();
		~UIPipeline();
		void createPipeline();
		void createRenderPass();
		void createDescriptorSet();
		void draw(frameData& fd, vk::Framebuffer framebuffer, uint32_t imageIndex);
		vk::DescriptorSetLayout getDescriptorSetLayout();
	private:
		vulkanShader m_vertexShader;
		vulkanShader m_fragShader;

		vk::UniquePipelineLayout m_PipelineLayout;
		vk::UniqueRenderPass m_UIRenderPass;
		vk::UniqueDescriptorSetLayout m_descriptorSetLayout;
		vk::UniqueDescriptorPool m_DescriptorPool;
		vk::UniqueDescriptorSet m_DescriptorSet;
		// pipeline
		vk::UniquePipeline m_pipeline;

		font m_defaultFont;

		glyphsRenderData m_glyphsRenderData;
	};
}