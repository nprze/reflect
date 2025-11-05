#include "camera.h"

#include "renderer_p/renderer.h"
#include "world_p/world.h"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include "world_p/components.h"
#include "world_p/ecs.h"
#include "input.h"
namespace rfct {
	static entity cameraEntity;
    static glm::mat4 projectionMatrix;

    void recalculateProjectionMatrix(cameraComponent& cam) {
        glm::mat4 screenRot = glm::rotate(glm::mat4(1), glm::radians(world::getWorld().screenViewTransformDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
        projectionMatrix = screenRot* glm::perspective(glm::radians(cam.fov), cam.aspectRatio, cam.nearPlane, cam.farPlane);
    }

    void setCamera(entity camera)
    {
		cameraEntity = camera;
		recalculateProjectionMatrix(ecs::get().get<cameraComponent>(camera));
    }
    void cameraComponentOnUpdate(float dt, entity player)
    {
        RFCT_PROFILE_SCOPE("camera update");
        /*
        if (input::getInput().cameraXAxis || input::getInput().cameraYAxis) {
            cameraEntity.get_mut<positionComponent>()->position.x += dt * input::getInput().cameraXAxis;
            cameraEntity.get_mut<positionComponent>()->position.y += dt * input::getInput().cameraYAxis;
        }
        else*/
            //glm::vec2 playerPos = { 2,7 };
        entt::registry& reg = ecs::get();
        glm::vec2 playerPos = reg.get<positionComponent>(player).position;

        // Move camera to player
        auto& camPos3D = reg.get<position3DComponent>(cameraEntity);
        camPos3D.position.x = playerPos.x;
        camPos3D.position.y = playerPos.y;

        // Handle framebuffer resize
        if (rfct::renderer::getRen().getRenderImagesManager().getSwapChain().framebufferResized) {
            auto& camComp = reg.get<cameraComponent>(cameraEntity);
            camComp.aspectRatio = renderer::getRen().getAspectRatio();
            recalculateProjectionMatrix(camComp);
        }

        // Always recalc projection
        recalculateProjectionMatrix(reg.get<cameraComponent>(cameraEntity));

    }

    const glm::mat4 flipY = glm::scale(glm::mat4(1.0f), glm::vec3(1, -1, 1));
    glm::mat4 getViewMatrix() {
        glm::mat4 model = glm::mat4(1.0f);
        position3DComponent& position = ecs::get().get<position3DComponent>(cameraEntity);
        model = glm::translate(model, (glm::vec3)position.position);
        rotationComponent& rotation = ecs::get().get<rotationComponent>(cameraEntity);

        glm::mat4 rotationMat = glm::yawPitchRoll(rotation.rotation.x, rotation.rotation.y, 0.f);
        glm::vec3 direction = glm::vec3(rotationMat * glm::vec4(0, 0, -1, 1));
        return flipY * glm::lookAt(position.position, position.position + direction, glm::vec3(0, 1, 0));
    }
    glm::mat4 getVPMatrix() {

        return projectionMatrix * getViewMatrix();
    }
}