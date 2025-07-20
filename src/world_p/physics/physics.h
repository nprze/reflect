#pragma once
#include <glm/glm.hpp>
#include "world_p/components.h"
namespace rfct{
	struct BVHnode {
		glm::vec2 min;
		glm::vec2 max;
		int left = -1;
		int right = -1;
		entity entity;
	};
	struct frameContext;
	// setup
	void createQueries(entity sceneEntity);
	void cleanupQueries();
	void buildStaticObjBVH();
	void buildDynamicObjBVH();
	template<typename T>
	void buildBVH(flecs::query<T> qr, std::vector<BVHnode>* BVHnodes);

	void buildDynamicBVH(flecs::query<dynamicBoxColliderComponent, positionComponent>& qr, std::vector<BVHnode>* BVHnodes);

	// update
	void updatePhysics(const frameContext* ctx);
}