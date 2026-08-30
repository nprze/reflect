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
        vk::Extent2D extent = renderer::getRen().getExtent();
		float aspectRatio = float(extent.width) / float(extent.height);
        projectionMatrix = glm::ortho(
            -8.f * aspectRatio, 8.f * aspectRatio,
			-8.f, 8.f,
            -100.0f,
            100.0f
        );
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

		// get size of camera view in world coords
        vk::Extent2D extent = renderer::getRen().getExtent();
		float aspectRatio = float(extent.width) / float(extent.height);
        constexpr float oneOverHundred = 1.f / 100.f;

		float visibleWidth = 16.f * aspectRatio;
        float visibleHeight = 16.f;

		glm::vec2 whereCameraShouldBe;
		if (visibleWidth / 2.f > sceneWidth - visibleWidth / 2.f) visibleWidth = 1.f; // case where scene is smaller than camera view
		if (visibleHeight / 2.f > sceneHeight - visibleHeight / 2.f) visibleHeight = 1.f;
        whereCameraShouldBe.x = std::clamp(playerPos.x, visibleWidth / 2.f, sceneWidth - visibleWidth / 2.f);
        whereCameraShouldBe.y = std::clamp(playerPos.y, visibleHeight / 2.f, sceneHeight - visibleHeight / 2.f);

		glm::vec2 directionToWhereCameraShouldBe = whereCameraShouldBe - glm::vec2(camPos3D.position.x, camPos3D.position.y);

        // Move camera to player
        float cameraAlignmentSpeed = 4.f + (camComp.screenShakeDuration / .07f) * 4.f;
        camPos3D.position.x+= 4.f * dt * directionToWhereCameraShouldBe.x;
        camPos3D.position.y+= 4.f * dt * directionToWhereCameraShouldBe.y;

        // screenShake- dash
        camComp.screenShakeDuration = std::clamp(camComp.screenShakeDuration - dt, 0.f, .07f);
        float screenShakeMultiplier = 0.f;
        if (camComp.screenShakeDuration > 0.f) {
            screenShakeMultiplier = (camComp.screenShakeDuration / .07f);
            float cameraMoveMultiplier = glm::length(whereCameraShouldBe - glm::vec2{ camPos3D.position.x, camPos3D.position.y });
            screenShakeMultiplier = (0.1f + cameraMoveMultiplier * 0.05f) * std::cos(3.14f * screenShakeMultiplier) * (1 - screenShakeMultiplier * screenShakeMultiplier);
        }
        camPos3D.position.x += camComp.screenShake.x * screenShakeMultiplier;
        camPos3D.position.y += camComp.screenShake.y * screenShakeMultiplier;

        // Always recalc projection
        recalculateProjectionMatrix(camComp);
    }

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