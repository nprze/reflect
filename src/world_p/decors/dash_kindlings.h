#pragma once
#include <glm/glm.hpp>

namespace rfct {
	void initKindlingsVars(scene* parentScene);
	void spawnKindling(RfctFrameContext* fc, const glm::vec2& position, const glm::vec2& playerVel, uint32_t var);
	void updateKindlings(RfctFrameContext* ctx);
	void updateKindlingMatrices(RfctFrameContext* ctx);
}