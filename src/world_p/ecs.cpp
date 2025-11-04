#include "ecs.h"

flecs::world rfct::ecs::world;

void rfct::ecs::recreateWorld()
{
	world.release();
	world = flecs::world();
}
