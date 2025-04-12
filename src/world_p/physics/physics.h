#pragma once
namespace rfct{
	void createQueries(entity sceneEntity);
	void cleanupQueries();
	void updatePhysics(float dt, entity sceneEntity);
}