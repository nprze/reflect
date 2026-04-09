#pragma once
#include "assets/serialize_structures/scene_serialize_data.h"

namespace rfct {
	struct objectSystem {
		virtual void initSystem() = 0;
		virtual void spawnData(scene* s, sceneSerializedData* sd) = 0;
		virtual void resetLevel(const frameContext* ctx) = 0;
		virtual void updateVisuals(const frameContext* ctx) = 0;
		virtual void updateSystem(frameContext* ctx) = 0;
		virtual void cleanupSystem() = 0;
	};
}