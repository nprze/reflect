#pragma once
#include "glm\glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include "world_p\world.h"
#include "input.h"
namespace rfct {
    void cameraComponentOnUpdate(float dt);

    inline glm::mat4 getViewMatrix(cameraComponent* cam)  {
        glm::mat4 rotationMat = glm::yawPitchRoll(cam->rotation.y, cam->rotation.x, cam->rotation.z);
        glm::vec3 direction = glm::vec3(rotationMat * glm::vec4(0, 0, -1, 1));
        return glm::lookAt(cam->position, cam->position + direction, glm::vec3(0, 1, 0));
    }

    inline glm::mat4 getProjectionMatrix(cameraComponent* cam) {
        return glm::perspective(glm::radians(cam->fov), cam->aspectRatio, cam->nearPlane, cam->farPlane);
    }

    inline static glm::mat4 getVPMatrix() {
        cameraComponent* cam = world::getWorld().getComponent<cameraComponent>(world::getWorld().camera);
        return getProjectionMatrix(cam) * getViewMatrix(cam);
    }
}