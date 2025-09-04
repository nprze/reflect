#include "death_anim.h"
#include "world_p/object_components.h"
#include "world_p/components.h"

namespace rfct {	
	static flecs::query<deathAnimParticle> kindlingParticlesComponentsQuery;
	static flecs::query<deathAnimParticle, dynamicSSBOIndexComponent> kindlingParticlesQuery;
}

void rfct::initPlayerDeathAnimVars()
{

}

void rfct::onPlayerDeath(frameContext* ctx)
{

}

bool rfct::updateDeathAnim(frameContext* ctx)
{

	return false;
}

void rfct::updateDeathAnimParticleMatrices()
{
}
