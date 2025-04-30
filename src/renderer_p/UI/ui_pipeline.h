#pragma once
#include "renderer_p/shader/vulkan_shader.h"
#include "renderer_p/frame/frame_data.h"
#include "font/font.h"

namespace rfct {
	struct glyphsRenderData {
		inline glyphsRenderData(uint32_t size) :buffer(size, vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU), bufferOffset(0), vertexCount(0) {
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
		vk::RenderPass& getRenderPass() { return m_UIRenderPass.get(); }
		void createPipeline();
		void createRenderPass();
		void createDescriptorSet();
		void draw(frameData& fd, vk::Framebuffer framebuffer);
		void debugText(const std::string& text, glm::vec2 startPosition, float scale);
		void addTextVertices(glyphsRenderData* rd, const std::string& text, glm::vec2 position, float scale, font* f = nullptr);
		int getTextureIndex(bindableImage* image);
		void addImage(const glm::vec2& min, const glm::vec2& max, bindableImage* image);
		vk::DescriptorSetLayout getDescriptorSetLayout();
	private:
		vulkanShader m_vertexShader;
		vulkanShader m_fragShader;

		vk::UniquePipelineLayout m_PipelineLayout;
		vk::UniqueRenderPass m_UIRenderPass;
		vk::UniqueDescriptorSetLayout m_descriptorSetLayout;
		vk::UniqueDescriptorPool m_DescriptorPool;
		vk::UniqueDescriptorSet m_DescriptorSet;
		std::unordered_map<bindableImage*, int> m_textureIndexMap;

		// pipeline
		vk::UniquePipeline m_pipeline;

		font m_defaultFont;

		glyphsRenderData m_glyphsRenderData;
		glyphsRenderData m_debugDrawglyphsRenderData;
	};
}