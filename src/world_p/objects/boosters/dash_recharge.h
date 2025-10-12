#pragma once
#include "context.h"
#include "assets/scene_serialize_data.h"

namespace rfct {
	void initDashRechargeVars(scene* parentScene, sceneSerializedData* sd);
	void updateDashRecharges(frameContext* ctx);
	void cleanupDashRechargeVars();
}