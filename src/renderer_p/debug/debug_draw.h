#pragma once
#include <glm/glm.hpp>
#include "renderer_p/buffer/vulkan_buffer.h"
#include "renderer_p/frame/frame_data.h"

namespace rfct {
	class RfctSwapChain;
	class RfctShader;
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
	struct debugTriangle {
		SmallVertex vertices[3];
	};
	struct debugLine {
		SmallVertex vertices[2];
	};
	struct debugDrawVertexBuffer {
		debugDrawVertexBuffer(uint32_t size);
		~debugDrawVertexBuffer();
		void postFrame() { bufferOffset = 0; vertexCount = 0; };
		VulkanBuffer buffer;
		uint32_t bufferOffset;
		uint32_t vertexCount;
		void* bufferMappedMemory;
	};

	class debugDraw {
	public:
		static debugTriangle* requestTriangles(uint32_t count);
		static debugLine* requestLines(uint32_t count);
		static float drawText(const std::string& text, glm::vec2 startPosition, float scale);
		static void flush(frameContext* ctx, frameData& fd, vk::Framebuffer framebuffer, vk::RenderPass renderPass);
	private:
		debugDraw(vk::RenderPass renderPass, vk::Device device);
		void draw(frameContext* ctx, RfctSwapChain& swapChain, frameData& fd, vk::Framebuffer framebuffer, vk::RenderPass renderPass);
		debugTriangle* requestNTriangles(uint32_t count);
		debugLine* requestNLines(uint32_t count);
		float text(UIPipelines& uiPipeline, const std::string& text, glm::vec2 startPosition, float scale);
	private:
		void createDebugPipelines(vk::RenderPass renderPass, vk::Device device);
	private:
		debugDrawVertexBuffer m_triangleBuffer;
		debugDrawVertexBuffer m_lineBuffer;
		RfctShader* m_vertexShader;
		RfctShader* m_fragShader;
		vk::UniquePipelineLayout m_PipelineLayout;
		// Triangle pipeline
		vk::UniquePipeline m_trianglePipeline;
		// Lines pipeline
		vk::UniquePipeline m_linePipeline;

		friend class RfctRenderer;
	};
}