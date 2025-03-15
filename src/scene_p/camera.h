#pragma once
#include "glm\glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"

namespace rfct {
	class camera {
    public:
        glm::vec3 position;
        glm::vec3 rotation;
        float fov, aspectRatio, nearPlane, farPlane;

        inline camera(glm::vec3 startPos, glm::vec3 startRot, float fov, float aspect, float nearP, float farP)
            : position(startPos), rotation(startRot), fov(fov), aspectRatio(aspect), nearPlane(nearP), farPlane(farP) {
        }

        inline glm::mat4 getVPMatrix() const {
            return getProjectionMatrix() * getViewMatrix();
        }

    private:
         inline glm::mat4 getViewMatrix() const {
            glm::mat4 rotationMat = glm::yawPitchRoll(rotation.y, rotation.x, rotation.z);
            glm::vec3 direction = glm::vec3(rotationMat * glm::vec4(0, 0, -1, 1));
            return glm::lookAt(position, position + direction, glm::vec3(0, 1, 0));
        }

         inline glm::mat4 getProjectionMatrix() const {
            return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
        }
	};
}