#pragma once
#include "context.h"
#include <glm/glm.hpp>

namespace rfct {
	void initSmokeVars();
	//void spawnFire(frameContext* ctx, const glm::vec2& position, const glm::vec2& direction = {0, 1}, float particlePerSec = 3, float lifetimeSec = 5.f, float lifetimeParticleSec = 1.f);
	entity spawnSmoke(frameContext* fc, const glm::vec2& position, const glm::vec2& direction = { 0, 1 }, uint32_t particleCount = 4, float lifetimeSec =1.f);
	void updateSmokes(frameContext* ctx);
}