#pragma once
#include "glm\glm.hpp"
namespace rfct {
	void setCamera(entity camera);
	void cameraComponentOnUpdate(float dt, entity player);
    glm::mat4 getVPMatrix();
}