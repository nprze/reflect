#pragma once
#include "components.h"
#include <glm/gtc/matrix_transform.hpp>
namespace rfct {
	static glm::mat4 getModelMatrixFromTranform(const transformComponent& tc) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, tc.position);
        model = glm::rotate(model, tc.rotation.x, glm::vec3(1, 0, 0));
        model = glm::rotate(model, tc.rotation.y, glm::vec3(0, 1, 0));
        model = glm::rotate(model, tc.rotation.z, glm::vec3(0, 0, 1));
        model = glm::scale(model, tc.scale);
        return model;

	}
}