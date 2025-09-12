#pragma once
#include "context.h"

namespace rfct {
	struct sceneSerializedData;
	struct scene;
	void spawnEnemies(sceneSerializedData* sc, scene* parent);
	void updateEnemies(frameContext* ctx);
}