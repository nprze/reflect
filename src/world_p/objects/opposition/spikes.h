#pragma once
#include "world_p/objects/object_system.h"
namespace rfct {
	struct spikes : objectSystem {
		void initSystem();
		void createQueries() {};
		void deleteQueries() {};
		void spawnData(scene* s, sceneSerializedData* sd);
		void resetLevel(const frameContext* ctx) {};
		void updateVisuals(const frameContext* ctx) {};
		void updateSystem(frameContext* ctx) {};
		void cleanupSystem() {};
	};
}