#pragma once
#include <glm/glm.hpp>
#include "renderer_p/buffer/vulkan_buffer.h"
#include "renderer_p/frame/frame_data.h"
#include "renderer_p/shader/vulkan_shader.h"
#include "context.h"
namespace rfct {
	struct SmallVertex {
		glm::vec3 pos;
		glm::vec3 color;

		static vk::VertexInputBindingDescription getBindingDescription() {
			vk::VertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(SmallVertex);
			bindingDescription.inputRate = vk::VertexInputRate::eVertex;
			return bindingDescription;
		}

		static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
			std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
			attributeDescriptions[0].offset = offsetof(SmallVertex, pos);
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
			attributeDescriptions[1].offset = offsetof(SmallVertex, color);
			return attributeDescriptions;
		}
	};
	// helpers
	struct debugTriangle {
		SmallVertex vertices[3];
	};
	struct debugLine {
		SmallVertex vertices[2];
	};

	struct debugDrawVertexBuffer{
		debugDrawVertexBuffer(uint32_t size) :buffer(size, vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU), bufferOffset(0), vertexCount(0) {
			bufferMappedMemory = buffer.Map();
		};
		~debugDrawVertexBuffer() { buffer.Unmap(); };
		void postFrame() { bufferOffset = 0; vertexCount = 0; };
		VulkanBuffer buffer;
		uint32_t bufferOffset;
		uint32_t vertexCount;
		void* bufferMappedMemory;
	};

	class debugDraw {
	private:
		static debugDraw* instance;
	public:
		static debugTriangle* requestTriangles(uint32_t count) { return instance->requestNTriangles(count); };
		static debugLine* requestLines(uint32_t count) { return instance->requestNLines(count); };
		static void drawText(const std::string& text, glm::vec2 startPosition, float scale) { instance->text(text, startPosition, scale); }
		static void flush(frameContext* ctx, frameData& fd, vk::Framebuffer framebuffer) { instance->draw(ctx, fd, framebuffer); };
	private:
		debugDraw();
		~debugDraw();
		void draw(frameContext* ctx, frameData& fd, vk::Framebuffer framebuffer);
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