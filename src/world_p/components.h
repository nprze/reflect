#pragma once
#include <string>
#include <glm\glm.hpp>
#include "renderer_p\buffer\vulkan_buffer.h"
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

    void updateDynamicRenderMeshComponents(scene* scene, frameContext* ctx);
    void updatePlayer(scene* scene, flecs::entity* player, float dt);
    void updatePhysics(scene* scene, float dt);


	struct sceneComponent {
		std::string name;
	};
    struct cameraComponent {
        float fov, aspectRatio, nearPlane, farPlane;
    };

    struct positionComponent {
        glm::vec3 position = glm::vec3(0.f);
    };
    struct rotationComponent {
        glm::vec3 rotation = glm::vec3(0.f);
    };
    struct scaleComponent {
        glm::vec3 scale = glm::vec3(1.f);
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


    struct staticBoxColliderComponent {
        glm::vec3 min;
        glm::vec3 max;
    };
    struct dynamicBoxColliderComponent {
        glm::vec3 min;
        glm::vec3 max;
    };


	struct velocityComponent {
		glm::vec3 velocity;
	};
	struct gravityComponent {
		bool gravityEnabled = true;
		float gravity = 10.f;
	};

    struct transform {
		positionComponent pos;
        rotationComponent rot;
		scaleComponent scale;
    };
}