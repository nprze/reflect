#pragma once
#include "job_system_p/job_system.h"

namespace rfct {
	enum gameState {
		undefined,
		gameplay,
		stateDialogue,
		menu
	};
	class scene;
	class RfctFrameGraph;
	// structure to be passed around as update context
	struct frameContext {
		float dt; // delta time
		float globalTime;
		bool renderDebugDraw = false; // should debugdraw be rendered
		size_t frameInFlightIndex; // number <0, RFCT_FRAMES_IN_FLIGHT) of the frame which will be updated. this will be used to get the actual frame in flight resources 
		scene* scene; // scene which will be updated
		float imageWidth;
		float imageHeight;
		gameState state;
		RfctFrameGraph* frameGraph;
		jobTracker wholeUpdateTracker; // tracks jobs for whole system updates 
	};
}