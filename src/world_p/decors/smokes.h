#pragma once
#include <glm/glm.hpp>

namespace rfct {
	void initSmokeVars(scene* parentScene);
	void spawnSmoke(frameContext* fc, const glm::vec2& position, const glm::vec2& direction = { 0, 1 }, uint32_t particleCount = 6, float lifetimeSec =1.f);
	void setColors(float* is);
	void updateSmokes(frameContext* ctx);
	void updateSmokeMatrices(frameContext* ctx);
}