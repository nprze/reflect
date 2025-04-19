#pragma once
#include "flecs/flecs.h"
namespace rfct {
	class ecs { // TODO: make this thread safe
	public:
		inline static flecs::world& get()
		{
			return world;
		}
	private:
		static flecs::world world;
	};
}