#pragma once
#include "context.h"

namespace rfct {
	void initGame();
	void updateGameDynamic(frameContext& ContextArg); // updates things based on delta time
	void updateGameFixed(frameContext& ContextArg);   // updates things based on fixed delta time
	void cleanupGame();
}