#pragma once
#include "context.h"

enum UIPartsNames;
namespace rfct {
	gameState getState();
	void updateLastState(gameState newState);
	void defineUI();
	void switchUIPart(UIPartsNames part);
	void drawUI(frameContext* ctx);
}