#pragma once
#include "context.h"
#include "assets/scene_serialize_data.h"

namespace rfct {
	void initJumpBoosterVars(scene* parentScene, sceneSerializedData* sd);
	void updateJumpBoosters(frameContext* ctx);
	void cleanupJumpBoosterVars();
}