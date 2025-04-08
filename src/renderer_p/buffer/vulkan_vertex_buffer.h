#pragma once
#include "vulkan_buffer.h"
#include "renderer_p\rasterizer_pipeline\vertex.h"

namespace rfct {
	class vulkanVertexBuffer {
	public:
		vulkanVertexBuffer() = default;
		vulkanVertexBuffer(vk::DeviceSize);
		~vulkanVertexBuffer();

		inline const vk::Buffer& getBuffer() { return m_Buffer.buffer; }
		size_t copyData(std::vector<Vertex> vertices);

		VulkanBuffer m_Buffer;

	private:
		size_t m_BufferOffset = 0;
	};
}
