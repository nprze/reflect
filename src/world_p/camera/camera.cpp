#include "camera.h"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include "input.h"
#include "world_p/world.h"
#include "world_p/components.h"
#include "renderer_p/renderer.h"

namespace rfct {
	static entity cameraEntity;
    static glm::mat4 projectionMatrix;
    const glm::mat4 flipY = glm::scale(glm::mat4(1.0f), glm::vec3(1, -1, 1));
	float fov = 45.f;

    void recalculateProjectionMatrix(cameraComponent& cam) {
        RFCT_PROFILE_FUNCTION();
        glm::mat4 screenRot = glm::rotate(glm::mat4(1), glm::radians(world::getWorld().screenViewTransformDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
        projectionMatrix = screenRot* glm::perspective(glm::radians(cam.fov), cam.aspectRatio, cam.nearPlane, cam.farPlane);
    }

    void setCamera(entity camera) {
		cameraEntity = camera;
		recalculateProjectionMatrix(ecs::get().get<cameraComponent>(camera));
    }

    void cameraComponentOnUpdate(float dt, entity player, int sceneWidth, int sceneHeight) {
        RFCT_PROFILE_SCOPE("camera update");
        entt::registry& reg = ecs::get();
        glm::vec2 playerPos = reg.get<positionComponent>(player).position;
        auto& camPos3D = reg.get<position3DComponent>(cameraEntity);
        auto& camComp = reg.get<cameraComponent>(cameraEntity);

        // Handle framebuffer resize
        if (rfct::renderer::getRen().getRenderImagesManager().getSwapChain().framebufferResized) {
            camComp.aspectRatio = renderer::getRen().getAspectRatio();
            recalculateProjectionMatrix(camComp);
        }
		// get size of camera view in world coords
        float distance = std::abs(camPos3D.position.z - 0.0f);
        float fovRad = glm::radians(camComp.fov);

        float visibleHeight = 2.0f * distance * std::tan(fovRad * 0.5f);
        float visibleWidth = visibleHeight * camComp.aspectRatio;

		glm::vec2 whereCameraShouldBe;
		if (visibleWidth / 2.f > sceneWidth - visibleWidth / 2.f) visibleWidth = 1.f; // case where scene is smaller than camera view
		if (visibleHeight / 2.f > sceneHeight - visibleHeight / 2.f) visibleHeight = 1.f;
        whereCameraShouldBe.x = std::clamp(playerPos.x, visibleWidth / 2.f, sceneWidth - visibleWidth / 2.f);
        whereCameraShouldBe.y = std::clamp(playerPos.y, visibleHeight / 2.f, sceneHeight - visibleHeight / 2.f);

		glm::vec2 directionToWhereCameraShouldBe = whereCameraShouldBe - glm::vec2(camPos3D.position.x, camPos3D.position.y);

        // Move camera to player
        camPos3D.position.x+= 4.f * dt * directionToWhereCameraShouldBe.x;
        camPos3D.position.y+= 4.f * dt * directionToWhereCameraShouldBe.y;

        // Always recalc projection
        recalculateProjectionMatrix(reg.get<cameraComponent>(cameraEntity));
    }
    glm::mat4 getViewMatrix() {
        glm::mat4 model = glm::mat4(1.0f);
        position3DComponent& position = ecs::get().get<position3DComponent>(cameraEntity);
        model = glm::translate(model, (glm::vec3)position.position);
        rotationComponent& rotation = ecs::get().get<rotationComponent>(cameraEntity);

        glm::mat4 rotationMat = glm::yawPitchRoll(rotation.rotation.x, rotation.rotation.y, 0.f);
        glm::vec3 direction = glm::vec3(rotationMat * glm::vec4(0, 0, -1, 1));
        return flipY * glm::lookAt (position.position, position.position + direction, glm::vec3(0, 1, 0));
    }
    glm::mat4 getVPMatrix() {

        return projectionMatrix * getViewMatrix();
    }
}