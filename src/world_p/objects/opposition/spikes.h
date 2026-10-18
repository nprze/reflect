#pragma once
#include "world_p/objects/object_system.h"

namespace rfct {
	struct spikes : objectSystem {
		void initSystem();
		void spawnData(scene* s, sceneSerializedData* sd);
		void resetLevel(const RfctFrameContext* ctx) {};
		void updateVisuals(const RfctFrameContext* ctx) {};
		void updateSystem(RfctFrameContext* ctx) {};
		void cleanupSystem() {};
	};
}