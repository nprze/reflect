#pragma once
#include "context.h"

#include "assets/frame_animation.h"

namespace rfct {
	struct sceneSerializedData;
	struct animationBuffer;
	struct scene;
	void spawnEnemies(sceneSerializedData* sc, scene* parent, animationBuffer* animBuffer);
	void updateEnemies(frameContext* ctx);
	void cleanupEnemies();
	void updateEnemiesMatrices(frameContext* ctx);
	void drawEnemies(vk::CommandBuffer& cmd, frameContext* ctx);
}