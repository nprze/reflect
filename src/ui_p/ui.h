#pragma once
#include "context.h"

namespace rfct {
	gameState getState();
	void updateLastState(gameState newState);
	void drawUI(frameContext* ctx);
}