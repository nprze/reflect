#include "camera.h"

#include "renderer_p\renderer.h"
#include "world_p\world.h"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include "world_p\components.h"
#include "world_p\ecs.h"
#include "input.h"
namespace rfct {
	static entity cameraEntity;
    static glm::mat4 projectionMatrix;

    void recalculateProjectionMatrix(const cameraComponent* cam) {
        projectionMatrix = glm::perspective(glm::radians(cam->fov), cam->aspectRatio, cam->nearPlane, cam->farPlane);
    }

    void setCamera(entity camera)
    {
		cameraEntity = camera;
		recalculateProjectionMatrix(camera.get<cameraComponent>());
    }
    void cameraComponentOnUpdate(float dt, entity player)
    {
        if (input::getInput().cameraXAxis || input::getInput().cameraYAxis) {
            cameraEntity.get_mut<positionComponent>()->position.x += dt * input::getInput().cameraXAxis;
            cameraEntity.get_mut<positionComponent>()->position.y += dt * input::getInput().cameraYAxis;
        }
        else {
			glm::vec2 playerPos = player.get<positionComponent>()->position;
            cameraEntity.get_mut<position3DComponent>()->position.x = playerPos.x;
            cameraEntity.get_mut<position3DComponent>()->position.y = playerPos.y;
        }
        if (renderer::getRen().getDeviceWrapper().getSwapChain().framebufferResized) {
            cameraEntity.get_mut<cameraComponent>()->aspectRatio = renderer::getRen().getAspectRatio();
			recalculateProjectionMatrix(cameraEntity.get<cameraComponent>());
        }

    }
    glm::mat4 getViewMatrix() {
        glm::mat4 model = glm::mat4(1.0f);
        const position3DComponent* position = cameraEntity.get<position3DComponent>();
        model = glm::translate(model, (glm::vec3)position->position);
        const rotationComponent* rotation = cameraEntity.get<rotationComponent>();

        glm::mat4 rotationMat = glm::yawPitchRoll(rotation->rotation.x, rotation->rotation.y, 0.f);
        glm::vec3 direction = glm::vec3(rotationMat * glm::vec4(0, 0, -1, 1));
        return glm::lookAt(position->position, position->position + direction, glm::vec3(0, 1, 0));
    }
    glm::mat4 getVPMatrix() {

        return projectionMatrix * getViewMatrix();
    }
}