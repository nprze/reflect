#include "app.h"
#include "world_p/world.h"
#include "game.h"
#include "ui_p/ui.h"
#include "world_p/progress/user_progress.h"

bool rfct::reflectApplication::isAppMinimised;

rfct::reflectApplication::reflectApplication(RFCT_APP_ARGS):
m_Renderer(RFCT_RENDERER_ARGUMENTS_VAR)
{
	// app init
	input::getInput().init();
	userSettings().loadUserSettings();
	isAppMinimised = false;
	initGame();
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
	cleanupGame();
}

void rfct::reflectApplication::update() {
	// delta time
	static auto previousTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> deltaTime = currentTime - previousTime;
	previousTime = currentTime;

	// context utils
	currentFrame = (currentFrame + 1) % RFCT_FRAMES_IN_FLIGHT;
	frameContext context = {
		.dt = deltaTime.count(),
		.frame = currentFrame,
		.scene = &world::getWorld().getCurrentScene(),
		.state = getState()
	};
	static float accululator = 0.f;
	accululator += context.dt;
	while (accululator >= fixedDeltaTime) {
		accululator -= fixedDeltaTime;
		context.fixedUpdateTimes++;
	}

	input::getInput().pollAndParseEvents(&context);
	if (!isAppMinimised) {
		updateGame(context);
		renderer::getRen().render(context);
	};
	updateLastState(context.state);
}
