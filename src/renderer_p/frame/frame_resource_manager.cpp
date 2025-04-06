#include "frame_resource_manager.h"
#include "renderer_p\frame\frame_data.h"
#include "renderer_p\renderer.h"
namespace rfct {
	framesInFlight::framesInFlight()
	{
		m_frames.reserve(RFCT_FRAMES_IN_FLIGHT);
		m_presentFinishedSemaphores.reserve(RFCT_FRAMES_IN_FLIGHT);
		for (uint32_t i = 0; i < RFCT_FRAMES_IN_FLIGHT; i++) {
			m_presentFinishedSemaphores.push_back(renderer::getRen().getDevice().createSemaphoreUnique({}));
		}
		for (uint32_t i = 0; i < RFCT_FRAMES_IN_FLIGHT; i++) {
			m_frames.push_back(std::make_unique<frameData>(renderer::getRen().getDevice(), renderer::getRen().getAllocator(), (m_presentFinishedSemaphores[(i+(RFCT_FRAMES_IN_FLIGHT-1)) % RFCT_FRAMES_IN_FLIGHT]).get()));
			RFCT_TRACE("Frame in flight #{0} created", i);
		}
	}

	framesInFlight::~framesInFlight()
	{
		cameraUbo::destroyDescriptorSetLayout();
	}

	frameData& framesInFlight::getNextFrame(frameContext* ctx)
	{
		frameData& fc = *m_frames[m_nextFrame].get();
		m_nextFrame = (m_nextFrame + 1) % m_frames.size();
		return fc;
	}
}