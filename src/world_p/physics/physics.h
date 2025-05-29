#pragma once
namespace rfct{
	struct frameContext;
	// setup
	void createQueries(entity sceneEntity);
	void cleanupQueries();
	void buildBVH(); // for static entities

	// update
	void updatePhysics(const frameContext* ctx);
}