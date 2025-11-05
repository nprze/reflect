#pragma once
#include <glm/glm.hpp>
#include "world_p/components.h"

namespace rfct{
	struct nearestObject {
		entity object = entity();
		glm::vec2 closestPosition = { 0,0 };
		int vineIndex = -1; // -2 if block, -1 if no object found
	};
	struct BVHnode {
		glm::vec2 min;
		glm::vec2 max;
		int left = -1;
		int right = -1;
		entity entity;
	};
	extern std::vector<BVHnode> StaticObjsBVHnodes;
	extern std::vector<BVHnode> DynamicObjsBVHnodes;
	struct frameContext;
	// setup
	void buildStaticObjBVH();
	void buildDynamicObjBVH();
	entity findTheNearestVineToPlayer(entity player);
	entity findTheNearestBlockToPlayer(entity player);
	void drawAABB(const glm::vec2& min, const glm::vec2& max, uint32_t depth);
	void buildStaticBVH(std::vector<BVHnode>* BVHnodes);
	glm::vec2 nearestPointOnAABB(const glm::vec2& point, const glm::vec2& AABBMin, const glm::vec2& AABBMax);
	void buildDynamicBVH(std::vector<BVHnode>* BVHnodes);

	// update
	void updatePhysics(const frameContext* ctx);
}