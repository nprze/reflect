#pragma once
#include "context.h"

enum UIPartsNames;
namespace rfct {
	gameState getState();
	void updateLastState(gameState newState);

	void defineUI();
	void drawUI(frameContext* ctx);
}