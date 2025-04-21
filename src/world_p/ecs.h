#pragma once
#include "flecs/flecs.h"
namespace rfct {
	class ecs {
	public:
		inline static flecs::world& get()
		{
			return world;
		}
	private:
		static flecs::world world;
	};
}