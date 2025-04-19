#pragma once
namespace rfct{
	// setup
	void createQueries(entity sceneEntity);
	void cleanupQueries();
	void buildBVH(); // for static entities

	// update
	void updatePhysics(float dt);
}