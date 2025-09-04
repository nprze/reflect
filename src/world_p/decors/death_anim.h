#pragma once
#include "context.h"

namespace rfct {
	void initPlayerDeathAnimVars();
	void onPlayerDeath(frameContext* ctx);
	bool updateDeathAnim(frameContext* ctx); // rerturns if finished playing
	void updateDeathAnimParticleMatrices();
}