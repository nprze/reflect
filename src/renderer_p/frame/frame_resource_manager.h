#pragma once
#include "renderer_p/frame/frame_data.h"

namespace rfct {
	class RfctFrameInFlight {
	public:
		RfctFrameInFlight(RfctVulkanMemAllocator& allocator, RfctQueue& queue, vk::Device device);
		RfctFrameSyncData& GetNextFrame(uint32_t frame_index) { return *m_frames[frame_index].get(); }
	private:
		uint32_t m_nextFrame = 0;
		std::vector<unique<RfctFrameSyncData>> m_frames;
		std::vector<vk::UniqueFence> m_fences;
	};
}