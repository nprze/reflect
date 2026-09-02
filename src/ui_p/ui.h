#pragma once

namespace rfct {
	class RfctSwapChain;
	gameState getState();
	void updateLastState(gameState newState);

	void defineUI();
	void drawUI(frameContext* ctx, RfctSwapChain& swapChain);
}