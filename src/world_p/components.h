#pragma once
#include <string>
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
        damageComponent = 1ULL << 1,
        healthComponent = 1ULL << 2,
    };

    struct ComponentBase {
	    virtual ComponentEnum getComponentEnum() = 0;
    };
	struct nameComponent : public ComponentBase {
        static constexpr ComponentEnum EnumValue = ComponentEnum::nameComponent;
        inline ComponentEnum getComponentEnum() override {
            return ComponentEnum::nameComponent;
        };
		std::string name;
	};
	struct damageComponent : public ComponentBase {
        static constexpr ComponentEnum EnumValue = ComponentEnum::damageComponent;
        inline ComponentEnum getComponentEnum() override {
            return ComponentEnum::damageComponent;
        };
		int damage;
	};
	struct healthComponent : public ComponentBase {
        static constexpr ComponentEnum EnumValue = ComponentEnum::healthComponent;
        inline ComponentEnum getComponentEnum() override {
            return ComponentEnum::healthComponent;
        };
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


    template <typename... Components>
    constexpr ComponentEnum GetComponentMask() {
        uint64_t mask = 0;
        ((mask |= (uint64_t)GetComponentEnum<Components>()), ...);
        return (ComponentEnum)mask;
    }

    template <typename T>
    constexpr ComponentEnum GetComponentEnum();

    template <>
    constexpr ComponentEnum GetComponentEnum<nameComponent>() {
        return ComponentEnum::nameComponent;
    }
    template <>
    constexpr ComponentEnum GetComponentEnum<damageComponent>() {
        return ComponentEnum::damageComponent;
    }
    template <>
    constexpr ComponentEnum GetComponentEnum<healthComponent>() {
        return ComponentEnum::healthComponent;
    }

}