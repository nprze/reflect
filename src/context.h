#pragma once
namespace rfct {
	class scene;
	struct frameContext {
		float dt; // delta time
		bool renderDebugDraw = false; // should debugdraw be rendered
		size_t frame; // number <0, RFCT_FRAMES_IN_FLIGHT-1> of the frame which will be updated. this will be used to get the actual frame in flight resources 
		scene* scene; // scene which will be updated
	};
}