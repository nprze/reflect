#include "vulkan_vertex_buffer.h"

rfct::vulkanVertexBuffer::vulkanVertexBuffer(vk::DeviceSize size) :m_Buffer(size, vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU)
{
}

rfct::vulkanVertexBuffer::~vulkanVertexBuffer()
{
}

void rfct::vulkanVertexBuffer::copyData(std::vector<Vertex> vertices)
{
	m_Buffer.CopyData(vertices.data(), vertices.size() * sizeof(Vertex));
}
