#pragma once

namespace rfct {
	gameState getState();
	void updateLastState(gameState newState);

	void defineUI();
	void drawUI(frameContext* ctx);
}