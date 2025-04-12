#pragma once
#include "components.h"
#include <glm/gtc/matrix_transform.hpp>
namespace rfct {
	static glm::mat4 getModelMatrixFromEntity(const flecs::entity& e) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3{ e.get<positionComponent>()->position , 0.f });
        glm::vec3 rotation = { e.get<rotationComponent>()->rotation, 0.f };
        model = glm::rotate(model, rotation.x, glm::vec3(1, 0, 0));
        model = glm::rotate(model, rotation.y, glm::vec3(0, 1, 0));
        model = glm::rotate(model, rotation.z, glm::vec3(0, 0, 1));
        model = glm::scale(model, {e.get<scaleComponent>()->scale, 0.f});
        return model;

	}
    static glm::mat4 getModelMatrixFromTransform(const transform& trans) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, { trans.pos.position, 0.f});
        glm::vec3 rotation = { trans.rot.rotation, 0.f };
        model = glm::rotate(model, rotation.x, glm::vec3(1, 0, 0));
        model = glm::rotate(model, rotation.y, glm::vec3(0, 1, 0));
        model = glm::rotate(model, rotation.z, glm::vec3(0, 0, 1));
        model = glm::scale(model, { trans.scale.scale, 0.f });
        return model;

    }
}