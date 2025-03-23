#pragma once
#include <string>
namespace rfct {
	struct Entity {
		size_t id;
	};


    enum class ComponentEnum : std::uint64_t {
        None = 0,
        nameComponent = 1ULL << 0,
    };


	struct NameComponent{
		std::string name;
	};
	struct DamageComponent {
		int damage;
	};
}