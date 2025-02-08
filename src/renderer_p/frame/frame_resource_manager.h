#pragma once
#include <vulkan\vulkan.hpp>
#include "renderer_p\frame\frame_data.h"
#include <vma\vk_mem_alloc.h>
#include <utils\ptr.h>
namespace rfct {
	class framesInFlight {
	public:
		framesInFlight();
		~framesInFlight();
		frameData& getNextFrame();
	private:
		VmaAllocator m_allocator;
		uint32_t m_nextFrame;
		std::vector<unique<frameData>> m_frames;
	};
}