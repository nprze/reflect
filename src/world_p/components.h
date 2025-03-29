#pragma once
#include <string>
#include <glm\glm.hpp>
namespace rfct {
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
        damageComponent = 1ULL << 2,
        healthComponent = 1ULL << 3,
    };

	struct nameComponent {
        nameComponent() = default;
		nameComponent(std::string n) : name(n) {}
        static constexpr ComponentEnum EnumValue = ComponentEnum::nameComponent;
		std::string name;
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
    struct cameraComponent {
        static constexpr ComponentEnum EnumValue = ComponentEnum::nameComponent;
        glm::vec3 position;
        glm::vec3 rotation; 
        float fov, aspectRatio, nearPlane, farPlane;
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