#pragma once
#include <glm/glm.hpp>

namespace rfct {
	void initKindlingsVars(scene* parentScene);
	void spawnKindling(frameContext* fc, const glm::vec2& position, const glm::vec2& playerVel, uint32_t var);
	void updateKindlings(frameContext* ctx);
	void updateKindlingMatrices(frameContext* ctx);
}