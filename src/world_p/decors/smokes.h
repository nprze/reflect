#pragma once
#include "context.h"
#include <glm/glm.hpp>

namespace rfct {
	void initSmokeVars(scene* parentScene);
	void cleanupSmokes();
	//void spawnFire(frameContext* ctx, const glm::vec2& position, const glm::vec2& direction = {0, 1}, float particlePerSec = 3, float lifetimeSec = 5.f, float lifetimeParticleSec = 1.f);
	void spawnSmoke(frameContext* fc, const glm::vec2& position, const glm::vec2& direction = { 0, 1 }, uint32_t particleCount = 6, float lifetimeSec =1.f);
	void setColors(float* is);
	void updateSmokes(frameContext* ctx);
	void updateSmokeMatrices(frameContext* ctx);
}