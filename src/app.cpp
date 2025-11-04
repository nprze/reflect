#include "app.h"
#include "world_p/world.h"
#include "assets/assets_manager.h"
#include "world_p/player/player_animations.h"

bool rfct::reflectApplication::isAppMinimised;

rfct::reflectApplication::reflectApplication(RFCT_APP_ARGS):
m_Renderer(RFCT_RENDERER_ARGUMENTS_VAR)
{
	// app init
	input::getInput().init();
	playerAnimations::get().loadAnimations();
	isAppMinimised = false;

	world::getWorld().initWorld("");

	lastState = gameState::gameplay;

#ifdef WINDOWS_BUILD
    update();
	renderer::getRen().getWindow().show();
	while (renderer::getRen().getWindow().pollAndParseEvents())
	{
		update();
	}
#endif
}
void rfct::reflectApplication::updateWindow(RFCT_APP_ARGS){
    m_Renderer.updateWindow(RFCT_NATIVE_WINDOW_ANDROID_VAR);
};
rfct::reflectApplication::~reflectApplication()
{
	RFCT_TRACE("app cleanup start");
	renderer::getRen().getDevice().waitIdle();
	world::getWorld().cleanWorld();
	playerAnimations::get().unloadAnimations();
}

void rfct::reflectApplication::update() {
	static auto previousTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> deltaTime = currentTime - previousTime;
	previousTime = currentTime;




	currentFrame = (currentFrame + 1) % RFCT_FRAMES_IN_FLIGHT;
	frameContext context = {
		.dt = deltaTime.count(),
		.frame = currentFrame,
		.scene = &world::getWorld().getCurrentScene(),
		.state = lastState
	};
	static float accululator = 0.f;
	accululator += context.dt;
	while (accululator >= fixedDeltaTime) {
		accululator -= fixedDeltaTime;
		context.fixedUpdateTimes++;
	}

	input::getInput().pollAndParseEvents(&context);
	if (!isAppMinimised) {
		updateGameplay(context);
		renderer::getRen().render(context);
	};
	lastState = context.state;
}

void rfct::reflectApplication::updateGameplay(frameContext& ContextArg)
{
	world::getWorld().onUpdate(ContextArg);
}
