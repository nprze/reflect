#pragma once
#include "world_p/components.h"
#include "renderer_p/buffer/vulkan_buffer.h"
namespace rfct {
	struct frameAnimation {

		uint32_t frameCount;
		float cycleTime;
		float timePerFrame;
		std::vector<uint32_t> trianglesPerFrame;

		VulkanBuffer* buffer;
		uint32_t bufferOffsetInBytes;
	};
}