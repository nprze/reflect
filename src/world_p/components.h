#pragma once
#include <glm/glm.hpp>

using collisionHandler = void(*)(entity, entity, glm::vec2);
using dynamicCollisionHandler = void(*)(entity, entity);
namespace flecs {
	class world;
	class entity;
}
namespace rfct {
    float randF();
    class scene;
	struct frameContext;

    enum class dynamicObjectType : uint8_t {
        Player = 0,
        Vine,
        Cigarette, 
        Smoke,
        NPC,
        Kindling,
        Spike
    };

    enum class playerState : uint8_t {
        normal = 0,
        dashing,
        jumping,
        holdingVines,
        holdingBlocks
    };





    struct objectLocation {
        uint32_t indexInSSBO;
        uint32_t verticesCount;
        size_t vertexBufferOffset;
    };

    void registerComponents();

    struct dynamicObjectTypeComponent {
        dynamicObjectType type;
    };
	struct sceneComponent {
		std::string name;
	};
    struct cameraComponent {
        float fov, aspectRatio, nearPlane, farPlane;
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
        float zAngularVelocity;
    };

    struct staticSSBOIndexComponent {
        uint32_t indexInSSBO;
    };
    struct dynamicSSBOIndexComponent {
        uint32_t indexInSSBO;
    };
    struct vertexRenderInfoComponent {
        uint32_t verticesCount;
        size_t vertexBufferOffset; // in vertices count (not bytes)
    };


    struct staticBoxColliderComponent { // it is in fact an AABB
        glm::vec2 min;
        glm::vec2 max;
    };
    struct dynamicBoxColliderComponent { // it is in fact an AABB
        glm::vec2 min;
        glm::vec2 max;
    };


	struct velocityComponent {
		glm::vec2 velocity;
	};

    struct inputVelocityComponent {
        glm::vec2 velocity;
    };
	struct gravityComponent {
        float oneMinusAirResistance = 0.97f;
		bool gravityEnabled = true;
		float gravity = 5.f;
	};

    struct playerStateComponent {
        bool grounded = false;
        bool holding = false;
        playerState  state = playerState::normal;
    };
    struct playerLifeComponent {
        bool alive = true;
    };
    struct playerDashStateComponent {
        bool dashing = false;
        float dashProgress = 0.f;
    };
    struct playerBoxToHoldComponent {
        staticBoxColliderComponent* boundingBox;
    };

    struct staticObjCollisionCallbackComponent {
        collisionHandler handler;
    };

    struct dynamicObjCollisionCallbackComponent {
        dynamicCollisionHandler handler;
    };

    struct transform {
		positionComponent pos;
        rotationComponent rot;
		scaleComponent scale;
    };
}