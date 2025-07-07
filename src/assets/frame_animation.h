#pragma once
#include "world_p/components.h"
#include "renderer_p/buffer/vulkan_buffer.h"
namespace rfct {
	struct animation {
		inline void init(VulkanBuffer* vb, uint32_t offset) {
			buffer = vb;
			bufferOffsetInBytes = offset;
		};

		uint32_t frameCount;
		float cycleTime;
		float timePerFrame;
		std::vector<uint32_t> trianglesPerFrame;

		VulkanBuffer* buffer;
		uint32_t bufferOffsetInBytes;
	};
}