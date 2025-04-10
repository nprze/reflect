#pragma once
#include <string>
#include <glm\glm.hpp>
#include "renderer_p\buffer\vulkan_buffer.h"
namespace rfct {
    class scene;
	struct frameContext;


    struct objectLocation {
        uint32_t indexInSSBO;
        uint32_t verticesCount;
        size_t vertexBufferOffset;
    };
	struct Entity {
		size_t id;
        operator size_t() const {
            return id;
        }
	};

    void updateDynamicRenderMeshComponents(scene* scene, frameContext* ctx);
    void updatePlayer(scene* scene, Entity player, float dt);
    void updatePhysics(scene* scene, float dt);



    enum class ComponentEnum : std::uint64_t {
        None = 0,
        nameComponent = 1ULL << 0,
        cameraComponent = 1ULL << 1,
        transformComponent = 1ULL << 2,
        staticRenderMeshComponent = 1ULL << 3,
        dynamicRenderMeshComponent = 1ULL << 4,
        staticBoxColliderComponent = 1ULL << 5,
        dynamicBoxColliderComponent = 1ULL << 6,
        rigidBodyComponent = 1ULL << 6,
        damageComponent = 1ULL << 20,
        healthComponent = 1ULL << 30,
    };

	struct nameComponent {
        nameComponent() = default;
		nameComponent(std::string n) : name(n) {}
        static constexpr ComponentEnum EnumValue = ComponentEnum::nameComponent;
		std::string name;
	};
    struct cameraComponent {
        static constexpr ComponentEnum EnumValue = ComponentEnum::cameraComponent;
        glm::vec3 position;
        glm::vec3 rotation;
        float fov, aspectRatio, nearPlane, farPlane;
    };
    struct transformComponent {
        static constexpr ComponentEnum EnumValue = ComponentEnum::transformComponent;
        glm::vec3 position = glm::vec3(0.f);
        glm::vec3 rotation = glm::vec3(0.f);
        glm::vec3 scale = glm::vec3(1.f);
    };
    struct staticRenderMeshComponent {
        static constexpr ComponentEnum EnumValue = ComponentEnum::staticRenderMeshComponent;
        objectLocation renderDataLocations;
    };
    struct dynamicRenderMeshComponent {
        static constexpr ComponentEnum EnumValue = ComponentEnum::dynamicRenderMeshComponent;
        objectLocation renderDataLocations;
    };
    struct staticBoxColliderComponent {
        static constexpr ComponentEnum EnumValue = ComponentEnum::staticBoxColliderComponent;
        glm::vec3 min;
        glm::vec3 max;
    };
    struct dynamicBoxColliderComponent {
        static constexpr ComponentEnum EnumValue = ComponentEnum::dynamicBoxColliderComponent;
        glm::vec3 min;
        glm::vec3 max;
    };
	struct rigidBodyComponent {
		static constexpr ComponentEnum EnumValue = ComponentEnum::rigidBodyComponent;
		glm::vec3 velocity;
		float gravityScale = 10.f;
	};
	struct damageComponent {
        damageComponent() = default;
        damageComponent(int dmg) : damage(dmg) {}
        static constexpr ComponentEnum EnumValue = ComponentEnum::damageComponent;
		int damage;
	};
	struct healthComponent {
        healthComponent() = default;
		healthComponent(int h) : health(h) {}
        static constexpr ComponentEnum EnumValue = ComponentEnum::healthComponent;
		int health;
	};














    constexpr ComponentEnum operator|(ComponentEnum lhs, ComponentEnum rhs) {
        return static_cast<ComponentEnum>(
            static_cast<std::uint64_t>(lhs) | static_cast<std::uint64_t>(rhs));
    }

    constexpr ComponentEnum operator&(ComponentEnum lhs, ComponentEnum rhs) {
        return static_cast<ComponentEnum>(
            static_cast<std::uint64_t>(lhs) & static_cast<std::uint64_t>(rhs));
    }

    constexpr ComponentEnum& operator|=(ComponentEnum& lhs, ComponentEnum rhs) {
        lhs = lhs | rhs;
        return lhs;
    }

    constexpr ComponentEnum operator&=(ComponentEnum lhs, ComponentEnum rhs) {
        lhs = lhs & rhs;
        return lhs;
    }

    constexpr bool operator==(ComponentEnum lhs, ComponentEnum rhs) {
		return static_cast<std::uint64_t>(lhs) == static_cast<std::uint64_t>(rhs);
    }

}