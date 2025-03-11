#pragma once
#include <glm\glm.hpp>
#include "renderer_p\buffer\vulkan_buffer.h"
#include "renderer_p\frame\frame_data.h"
#include "renderer_p\shader\vulkan_shader.h"
#include "renderer_p\rasterizer_pipeline\vertex.h"
#include <stdint.h>
namespace rfct {
	struct debugTriangle {
		Vertex vertices[3];
	};
	class debugDraw {
	private:

	public:
		inline static debugTriangle* requestTriangles(uint32_t count) { return instance->requestNTriangles(count); };
		inline static void flush(frameData& fd, vk::Framebuffer framebuffer, uint32_t imageIndex) { instance->draw(fd, framebuffer, imageIndex); };
	private:
		static debugDraw* instance;
	private:
		debugDraw();
		~debugDraw();
		void draw(frameData& fd, vk::Framebuffer framebuffer, uint32_t imageIndex);// valid usage: command buffer has been begun, not ended.
		debugTriangle* requestNTriangles(uint32_t count);

		
	private:
		VulkanBuffer m_Buffer;
		uint32_t m_Offset;
		uint32_t m_TriangleCount;
		void* m_MappedMemory;



		// Triangle pipeline
		void createTrianglePipeline();
		void createTriangleRenderPass();

		vk::UniquePipelineLayout m_trianglePipelineLayout;
		vk::UniquePipeline m_trianglePipeline;
		vk::UniqueRenderPass m_triangleRenderPass;
		vulkanShader m_triangleVertexShader;
		vulkanShader m_triangleFragShader;



		friend class renderer;
	};
}