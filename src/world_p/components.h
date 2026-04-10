#pragma once
#include <glm/glm.hpp>

using collisionHandler = void(*)(entity, entity, glm::vec2);
using dynamicCollisionHandler = void(*)(entity, entity); // the second entity must have dynamic box collider
using rayHitCallback = void(*)(entity, entity); // the second entity must have static box collider. the first one is the ray

namespace rfct {
    inline float randF() {
        static uint32_t seed = rand();
        seed = 1664525u * seed + 1013904223u;
        return (seed >> 8) * (1.0f / 16777216.0f);
    }

    enum class dynamicObjectType : uint8_t {
        Player = 0,
        Vine,
        Cigarette, 
        Smoke,
        NPC,
        Kindling,
        Spike, 
        Enemy,
        JumpBooster,
    };
    enum class playerState : uint8_t {
        normal = 0,
        dashing,
        jumping,
        holdingVines,
        holdingBlocks
    };

    struct objectLocation {
        uint32_t indexInSSBO = 0;
        uint32_t verticesCount = 0;
        size_t vertexBufferOffset = 0;
    };
    struct dynamicObjectTypeComponent {
        dynamicObjectType type = dynamicObjectType::Player;
        bool passable = true;
    };
    struct cameraComponent {
        float fov = 45.f;
        float aspectRatio = 1.f;
        float nearPlane = 0.1f;
        float farPlane = 100.f;
    };
    struct positionComponent {
        glm::vec2 position = glm::vec2(0.f);
    };
    struct position3DComponent {
        glm::vec3 position = glm::vec3(0.f);
    };
    struct rotationComponent {
        glm::vec3 rotation = glm::vec3(0.f);
    };
    struct scaleComponent {
        glm::vec2 scale = glm::vec2(1.f);
    };
    struct matrixComponent {
        glm::mat4 model = glm::mat4(1.f);
    };
    struct angularVelocityComponent {
        float zAngularVelocity = 0.f;
    };
    struct staticSSBOIndexComponent {
        uint32_t indexInSSBO = 0;
    };
    struct dynamicSSBOIndexComponent {
        uint32_t indexInSSBO = 0;
    };
    struct vertexRenderInfoComponent {
        uint32_t verticesCount = 0;
        size_t vertexBufferOffset = 0; // in vertices count (not bytes)
    };
    struct staticBoxColliderComponent { // it is in fact an AABB
        glm::vec2 min = glm::vec2(0.f);
        glm::vec2 max = glm::vec2(0.f);
    };
    struct dynamicBoxColliderComponent { // it is in fact an AABB
        glm::vec2 min = glm::vec2(0.f);
        glm::vec2 max = glm::vec2(0.f);
    };
	struct velocityComponent {
		glm::vec2 velocity = glm::vec2(0.f) ;
	};
    struct inputVelocityComponent {
        glm::vec2 velocity = glm::vec2(0.f);
    };
	struct gravityComponent {
        float oneMinusAirResistance = 0.97f;
		bool gravityEnabled = true;
		float gravity = 5.f;
	};
    struct playerStateComponent {
        bool grounded = false;
        bool allowToJump = false;
        playerState  state = playerState::normal;
        uint8_t dashCharges = 0;
    };
    struct playerLifeComponent {
        bool alive = true;
    };
    struct playerDashStateComponent {
        bool dashing = false;
        float dashProgress = 0.f;
    };
    struct staticObjCollisionCallbackComponent {
        collisionHandler handler = nullptr;
    };
    struct dynamicObjCollisionCallbackComponent {
        dynamicCollisionHandler handler = nullptr; 
    };
    struct transform {
		positionComponent pos;
        rotationComponent rot;
		scaleComponent scale;
    };
}