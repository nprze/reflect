#pragma once
#include <glm\glm.hpp>
#include "renderer_p\buffer\vulkan_buffer.h"
#include "renderer_p\frame\frame_data.h"
#include "renderer_p\shader\vulkan_shader.h"
#include "renderer_p\rasterizer_pipeline\vertex.h"
#include <stdint.h>
#include "context.h"
namespace rfct {
	struct debugTriangle {
		Vertex vertices[3];
	};
	struct debugLine {
		Vertex vertices[2];
	};
	struct debugDrawVertexBuffer{
		inline debugDrawVertexBuffer(uint32_t size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage) :buffer(size, usage, memoryUsage), bufferOffset(0), vertexCount(0) {
			bufferMappedMemory = buffer.Map();
		};
		inline ~debugDrawVertexBuffer() { buffer.Unmap(); };
		inline void postFrame() { bufferOffset = 0; vertexCount = 0; };
		VulkanBuffer buffer;
		uint32_t bufferOffset;
		uint32_t vertexCount;
		void* bufferMappedMemory;

		
	};
	class debugDraw {
	private:

	public:
		inline static debugTriangle* requestTriangles(uint32_t count) { return instance->requestNTriangles(count); };
		inline static debugLine* requestLines(uint32_t count) { return instance->requestNLines(count); };
		inline static void drawText(const std::string& text, glm::vec2 startPosition, float scale) { instance->text(text, startPosition, scale); }
		inline static void flush(frameContext* ctx, frameData& fd, vk::Framebuffer framebuffer, uint32_t imageIndex) { instance->draw(ctx, fd, framebuffer, imageIndex); };
	private:
		static debugDraw* instance;
	private:
		debugDraw();
		~debugDraw();
		void draw(frameContext* ctx, frameData& fd, vk::Framebuffer framebuffer, uint32_t imageIndex);
		debugTriangle* requestNTriangles(uint32_t count);
		debugLine* requestNLines(uint32_t count);
		void text(const std::string& text, glm::vec2 startPosition, float scale);

		
	private:
		debugDrawVertexBuffer m_triangleBuffer;
		debugDrawVertexBuffer m_lineBuffer;


		vulkanShader m_vertexShader;
		vulkanShader m_fragShader;



		// Pipelines
		void createPipelines();
		void createRenderPass();

		vk::UniquePipelineLayout m_PipelineLayout;
		vk::UniqueRenderPass m_debugDrawRenderPass;

		// Triangle pipeline
		vk::UniquePipeline m_trianglePipeline;

		// Lines pipeline
		vk::UniquePipeline m_linePipeline;



		friend class renderer;
	};
}