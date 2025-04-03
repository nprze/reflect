#pragma once
#include <string>
#include <glm\glm.hpp>
#include "renderer_p\buffer\vulkan_buffer.h"
namespace rfct {
    struct objectLocation {
        VulkanBuffer* SSBO;
        uint32_t indexInSSBO;
        uint32_t verticesCount;
        VulkanBuffer* vertexBuffer;
        size_t vertexBufferOffset;
    };
	struct Entity {
		size_t id;
        operator size_t() const {
            return id;
        }
	};


    enum class ComponentEnum : std::uint64_t {
        None = 0,
        nameComponent = 1ULL << 0,
        cameraComponent = 1ULL << 1,
        transformComponent = 1ULL << 2,
        renderMeshComponent = 1ULL << 3,
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
    struct renderMeshComponent {
        static constexpr ComponentEnum EnumValue = ComponentEnum::renderMeshComponent;
        objectLocation renderDataLocations;
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