#include "collision_callback.h"
#include "world_p/components.h"

void rfct::onCollision_Player_StaticObj(entity player, entity collidedWith, glm::vec2 resolution)
{
    positionComponent* pos = player.get_mut<positionComponent>();
    pos->position += resolution;


    velocityComponent* vel = player.get_mut<velocityComponent>();
    if (resolution.x != 0.0f) {
        vel->velocity.x = 0.0f;
    }
    if (resolution.y != 0.0f) {
        vel->velocity.y = 0.0f;
    }
	if (resolution.y < 0)
	{
		player.get_mut<playerStateComponent>()->grounded = true;
	}
}
