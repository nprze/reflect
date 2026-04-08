#pragma once
#include "renderer_p/buffer/vulkan_buffer.h"

namespace rfct {
	struct animationBuffer {
		void init(size_t bufferSizeInTriangles);
		void cleanupBuffer();
		vulkanBufferLocation requestTriangles(uint32_t triangleCount);
		vk::Buffer& getBuffer() { return buffer->buffer; };
	private:
		VulkanBuffer* buffer;
		size_t trianglesLeftInBuffer;
		size_t maxTriangles;
	};
}