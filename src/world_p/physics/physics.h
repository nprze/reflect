#pragma once
namespace rfct{
	void createQueries(entity sceneEntity);
	void buildBVH();
	void cleanupQueries();
	void updatePhysics(float dt);
}