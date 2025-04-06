#pragma once
#include <vulkan\vulkan.hpp>
#include "renderer_p\frame\frame_data.h"
#include <vma\vk_mem_alloc.h>
#include <utils\ptr.h>

namespace rfct {
	struct frameContext;
	class framesInFlight {
	public:
		framesInFlight();
		~framesInFlight();
		frameData& getNextFrame(frameContext* ctx);
	private:
		uint32_t m_nextFrame = 0;
		std::vector<unique<frameData>> m_frames;
		std::vector<vk::UniqueSemaphore> m_presentFinishedSemaphores;
	};
}