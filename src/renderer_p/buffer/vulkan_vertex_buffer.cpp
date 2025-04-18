#include "vulkan_vertex_buffer.h"

rfct::vulkanVertexBuffer::vulkanVertexBuffer(vk::DeviceSize size) :m_Buffer(size, vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU), m_BufferOffset(0)
{
}

rfct::vulkanVertexBuffer::~vulkanVertexBuffer()
{
}

size_t rfct::vulkanVertexBuffer::copyData(const std::vector<Vertex>& vertices)
{

	char* m_mappedData = (char*)m_Buffer.Map();
	m_mappedData += m_BufferOffset;

	std::memcpy(m_mappedData, vertices.data(), vertices.size() * sizeof(vertices[0]));

	m_Buffer.Unmap();
	m_BufferOffset += vertices.size() * sizeof(vertices[0]);
	return m_BufferOffset - vertices.size() * sizeof(vertices[0]);
}
