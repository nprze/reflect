#pragma once
#include "components.h"
#include <glm/gtc/matrix_transform.hpp>

namespace rfct {
	inline glm::mat4 getModelMatrixFromEntity(const entity& e) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3{ ecs::get().get<positionComponent>(e).position , 0.f });
        glm::vec3 rotation = ecs::get().get<rotationComponent>(e).rotation;
        model = glm::rotate(model, rotation.x, glm::vec3(1, 0, 0));
        model = glm::rotate(model, rotation.y, glm::vec3(0, 1, 0));
        model = glm::rotate(model, rotation.z, glm::vec3(0, 0, 1));
        model = glm::scale(model, { ecs::get().get<scaleComponent>(e).scale, 0.f});
        return model;
	}

    inline glm::mat4 getModelMatrix(const positionComponent& pos, const rotationComponent& rot, const scaleComponent& sc) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3{ pos.position , 0.f });
        glm::vec3 rotation = rot.rotation;
        model = glm::rotate(model, rotation.x, glm::vec3(1, 0, 0));
        model = glm::rotate(model, rotation.y, glm::vec3(0, 1, 0));
        model = glm::rotate(model, rotation.z, glm::vec3(0, 0, 1));
        model = glm::scale(model, { sc.scale, 0.f });
        return model;
    }

    inline glm::mat4 getModelMatrixFromTransform(const transform& trans) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, { trans.pos.position, 0.f});
        glm::vec3 rotation = trans.rot.rotation;
        model = glm::rotate(model, rotation.x, glm::vec3(1, 0, 0));
        model = glm::rotate(model, rotation.y, glm::vec3(0, 1, 0));
        model = glm::rotate(model, rotation.z, glm::vec3(0, 0, 1));
        model = glm::scale(model, { trans.scale.scale, 0.f });
        return model;
    }
}