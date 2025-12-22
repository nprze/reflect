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
	// structure to be passed around as update context
	struct frameContext {
		float dt; // delta time
		bool renderDebugDraw = false; // should debugdraw be rendered
		size_t frame; // number <0, RFCT_FRAMES_IN_FLIGHT-1> of the frame which will be updated. this will be used to get the actual frame in flight resources 
		scene* scene; // scene which will be updated
		gameState state;
		uint8_t fixedUpdateTimes;
		jobTracker wholeUpdateTracker; // tracks jobs for whole system updates 
	};
}