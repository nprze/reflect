#pragma once
#include "vulkan_buffer.h"
#include "renderer_p\rasterizer_pipeline\vertex.h"
namespace rfct {
	class vulkanVertexBuffer {
	public:
		vulkanVertexBuffer() = delete;
		vulkanVertexBuffer(vk::DeviceSize);
		~vulkanVertexBuffer();
		inline const vk::Buffer& getBuffer() { return m_Buffer.buffer; };
		void copyData(std::vector<Vertex> vertices);
	private:
		VulkanBuffer m_Buffer;
	};
}