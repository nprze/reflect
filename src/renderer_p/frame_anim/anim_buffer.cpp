#include "anim_buffer.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"

rfct::animationBuffer::animationBuffer()
{
	
}

void rfct::animationBuffer::init(size_t bufferSizeInTriangles)
{
	buffer = (VulkanBuffer*)malloc(sizeof(VulkanBuffer));
	new (buffer) VulkanBuffer("animation", bufferSizeInTriangles * 3 * sizeof(Vertex), vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, VMA_MEMORY_USAGE_GPU_ONLY);
	trianglesLeftInBuffer = bufferSizeInTriangles;
	maxTriangles = bufferSizeInTriangles;
}

void rfct::animationBuffer::cleanup()
{
	buffer->cleanup();
	free(buffer);
}

rfct::vulkanBufferLocation rfct::animationBuffer::requestTriangles(uint32_t triangleCount)
{
	if (trianglesLeftInBuffer >= triangleCount) {
		uint32_t val = static_cast<uint32_t>((3 * sizeof(Vertex) * (maxTriangles - trianglesLeftInBuffer)));
		trianglesLeftInBuffer -= triangleCount;
		return { buffer, val };
	}
	return { nullptr, 0 };
}
