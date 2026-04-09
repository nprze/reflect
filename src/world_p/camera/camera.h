#pragma once
#include "glm/glm.hpp"

namespace rfct {
	void setCamera(entity camera);
	void cameraComponentOnUpdate(float dt, entity player, int sceneWidth, int sceneHeight);
    glm::mat4 getVPMatrix();
}