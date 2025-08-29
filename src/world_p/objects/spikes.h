#pragma once
#include <glm/glm.hpp>
#include "context.h"
#include "assets/scene_serialize_data.h"

namespace rfct {
	void initSpikeVars(scene* parentScene);
	void cleanupSpikes();
	void createSpike(scene* parentScene, const SpikeInfo& spawnInfo);
}