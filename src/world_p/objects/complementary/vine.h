#pragma once
#include <glm/glm.hpp>
#include "renderer_p/rasterizer_pipeline/vertex.h"
#include "world_p/components.h"
#include "world_p/objects/object_system.h"

namespace rfct {
	struct nearestObject;
	std::pair<glm::vec2, int> getNearestEdgePos(const glm::vec2& PlayerPos, entity vine);
	glm::vec2 simulateVinePlayerIsHolding(entity player, entity vineEntity, int vineEdgeIndex, const frameContext* fc); // returns the player pos
	struct vines : objectSystem {
		void initSystem() {};
		void spawnData(scene* s, sceneSerializedData* sd);
		void resetLevel(const frameContext* ctx);
		void updateVisuals(const frameContext* ctx);
		void updateSystem(frameContext* ctx);
		void cleanupSystem() {};

		void onStartHolding(nearestObject& nearest);
		void onEndHolding();
	};
}