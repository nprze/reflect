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
	// collisions
	bool checkForCollisionAABBAABB(const glm::vec2& aMin, const glm::vec2& aMax, const glm::vec2& bMin, const glm::vec2& bMax);
	bool checkIntersectSegmentAABB(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& aabbMin, const glm::vec2& aabbMax);
	glm::vec2 ResolveAABBCollision(const dynamicBoxColliderComponent& dynamic, const staticBoxColliderComponent& staticCol);
	glm::vec2 ResolveAABBCollision(const glm::vec2& aMin, const glm::vec2& aMax, const glm::vec2& bMin, const glm::vec2& bMax); // returns what should be applied to object a to resolve :)
	bool checkRayStatic(const BVHnode& node, const glm::vec2& rayStart, const glm::vec2& rayEnd);
	glm::vec2 nearestPointOnAABB(const glm::vec2& point, const glm::vec2& AABBMin, const glm::vec2& AABBMax);
	// setup
	void buildStaticObjBVH();
	void buildDynamicObjBVH();
	entity findTheNearestVineToPlayer(const entity player);
	entity findTheNearestBlockToPlayer(const entity player);
	void buildStaticBVH(std::vector<BVHnode>* BVHnodes);
	void buildDynamicBVH(std::vector<BVHnode>* BVHnodes);
	// update
	void physicsStep(const frameContext* ctx);

	extern std::vector<BVHnode> StaticObjsBVHnodes;
	extern std::vector<BVHnode> DynamicObjsBVHnodes;
}