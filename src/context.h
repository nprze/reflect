#pragma once
namespace rfct {
	class scene;
	struct frameContext {
		float dt; // delta time
		size_t frame; // number <0, RFCT_FRAMES_IN_FLIGHT-1> of the frame which will be updated
		scene* scene; // scene which will be updated
		bool renderDebugDraw = false; // should debugdraw be rendered
	};
}