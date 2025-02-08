#include "frame_resource_manager.h"
#include "renderer_p\frame\frame_data.h"
#include "renderer_p\renderer.h"
namespace rfct {
	framesInFlight::framesInFlight()
	{
		VmaAllocatorCreateInfo allocatorCreateInfo = {};
		allocatorCreateInfo.physicalDevice = renderer::ren.getDeviceWrapper().getPhysicalDevice();
		allocatorCreateInfo.device = renderer::ren.getDevice();
		allocatorCreateInfo.instance = renderer::ren.getInstance();
		allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
		vmaCreateAllocator(&allocatorCreateInfo, &m_allocator);

		for (uint32_t i = 0; i < RFCT_FRAMES_IN_FLIGHT; i++) {
			m_frames.push_back(std::make_unique<frameData>(renderer::ren.getDevice(), m_allocator));
			RFCT_TRACE("Frame in flight #{0} created", i);
		}
	}

	framesInFlight::~framesInFlight()
	{
		vmaDestroyAllocator(m_allocator);
	}

	frameData& framesInFlight::getNextFrame()
	{
		frameData& fc = *m_frames[m_nextFrame].get();
		m_nextFrame = (m_nextFrame + 1) % m_frames.size();
		return fc;
	}
}