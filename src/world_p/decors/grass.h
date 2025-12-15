#pragma once
#include "context.h"

namespace rfct {
	struct sceneSerializedData;
	class scene;
	void initGrassVars(scene* parentScene);
	void spawnTallGrass(scene* parentScene, sceneSerializedData* sd);
	void cleanupGrass();
	void updateGrass(frameContext* ctx);
}