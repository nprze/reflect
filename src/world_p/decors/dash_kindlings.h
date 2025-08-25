#pragma once
#include "context.h"
#include <glm/glm.hpp>

namespace rfct {
	void initKindlingsVars(scene* parentScene);
	void cleanupKindlings();
	//void spawnFire(frameContext* ctx, const glm::vec2& position, const glm::vec2& direction = {0, 1}, float particlePerSec = 3, float lifetimeSec = 5.f, float lifetimeParticleSec = 1.f);
	void spawnKindling(frameContext* fc, const glm::vec2& position, const glm::vec2& playerVel, uint32_t var);
	void updateKindlings(frameContext* ctx);
	void updateKindlingMatrices(frameContext* ctx);
}