#pragma once
#include <glm\glm.hpp>
#include "renderer_p\buffer\vulkan_buffer.h"
#include "renderer_p\frame\frame_data.h"
#include "renderer_p\shader\vulkan_shader.h"
#include "renderer_p\rasterizer_pipeline\vertex.h"
namespace rfct {
	struct debugTriangle {
		Vertex vertices[3];
	};
	class debugDraw {
	public:
		inline static debugTriangle* requestTriangles(uint32_t count) { return instance->requestNTriangles(count); };
		inline static void flush(frameData& fd) { instance->draw(fd); };
	private:
		static debugDraw* instance;
	private:
		debugDraw();
		~debugDraw();
		void draw(frameData& fd);// valid usage: command buffer has been begun, not ended.
		debugTriangle* requestNTriangles(uint32_t count);

		
		void createTrianglePipeline();
	private:
		VulkanBuffer m_Buffer;
		uint32_t m_Offset;
		uint32_t m_TriangleCount;
		void* m_MappedMemory;


		// Pipeline
		/*
		vk::UniquePipelineLayout m_pipelineLayout;
		vk::UniquePipeline m_graphicsPipeline;
		vk::UniqueRenderPass m_renderPass;
		vulkanShader m_vertexShader;
		vulkanShader m_fragShader;*/


		friend class renderer;
	};
}