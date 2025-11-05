#pragma once
#include "entt/entt.hpp"
namespace rfct {
	class ecs {
	public:
		inline static entt::registry& get()
		{
			return registry;
		}
	private:
		static entt::registry registry;
	};
}