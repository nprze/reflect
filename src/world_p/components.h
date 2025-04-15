#pragma once
#include <glm\glm.hpp>

using collisionHandler = void(*)(entity, entity, glm::vec2);
namespace flecs {
	class world;
	class entity;
}
namespace rfct {
    class scene;
	struct frameContext;


    struct objectLocation {
        uint32_t indexInSSBO;
        uint32_t verticesCount;
        size_t vertexBufferOffset;
    };

    void registerComponents();



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
        glm::vec2 rotation = glm::vec2(0.f);
    };
    struct scaleComponent {
        glm::vec2 scale = glm::vec2(1.f);
    };
    struct matrixComponent {
        glm::mat4 model = glm::mat4(1.f);
    };

    struct staticSSBOIndexComponent {
        uint32_t indexInSSBO;
    };
    struct dynamicSSBOIndexComponent {
        uint32_t indexInSSBO;
    };
    struct vertexRenderInfoComponent {
        uint32_t verticesCount;
        size_t vertexBufferOffset;
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
	struct gravityComponent {
		bool gravityEnabled = true;
		float gravity = 10.f;
	};

    struct playerStateComponent {
        bool grounded = false;
    };

    struct collisionCallbackComponent {
        collisionHandler handler;
    };


    bool checkForCollisionAABBAABB(dynamicBoxColliderComponent* a, staticBoxColliderComponent* b);

    struct transform {
		positionComponent pos;
        rotationComponent rot;
		scaleComponent scale;
    };
}