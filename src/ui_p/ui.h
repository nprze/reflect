#pragma once

namespace rfct {
	class RfctSwapChain;
	gameState getState();
	void updateLastState(gameState newState);

	void defineUI();
	void drawUI(RfctFrameContext& ctx, RfctSwapChain& swapChain);
}