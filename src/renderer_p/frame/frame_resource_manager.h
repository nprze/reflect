#pragma once
#include "renderer_p/frame/frame_data.h"

namespace rfct {
	class framesInFlight {
	public:
		framesInFlight(RfctVulkanMemAllocator& allocator, RfctQueue& queue, vk::Device device);
		~framesInFlight();
		frameData& getNextFrame(uint32_t frame_index);
	private:
		uint32_t m_nextFrame = 0;
		std::vector<unique<frameData>> m_frames;
		std::vector<vk::UniqueFence> m_fences;
	};
}