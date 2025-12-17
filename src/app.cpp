#include "app.h"
#include "world_p/world.h"
#include "game.h"
#include "ui_p/ui.h"
#include "world_p/progress/user_progress.h"
#include <thread>

bool rfct::reflectApplication::isAppMinimised;

rfct::reflectApplication::reflectApplication(RFCT_APP_ARGS):
m_Renderer(RFCT_RENDERER_ARGUMENTS_VAR)
{
	// app init
	input::getInput().init();
	userSettings::get().loadUserSettings();
	isAppMinimised = false;
	initGame();
	defineUI();
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
    using clock = std::chrono::steady_clock;

    static auto previousTime = clock::now();
    auto frameStart = clock::now();

    // Delta time
    std::chrono::duration<float> deltaTime = frameStart - previousTime;
    previousTime = frameStart;

    currentFrame = (currentFrame + 1) % RFCT_FRAMES_IN_FLIGHT;
    frameContext context = {
        .dt = deltaTime.count(),
        .frame = currentFrame,
        .scene = &world::getWorld().getCurrentScene(),
        .state = getState()
    };

    static float accumulator = 0.f;
    accumulator += context.dt;
    while (accumulator >= fixedDeltaTime) {
        accumulator -= fixedDeltaTime;
        context.fixedUpdateTimes++;
    }

    input::getInput().pollAndParseEvents(&context);

    if (!isAppMinimised) {
        updateGame(context);
        renderer::getRen().render(context);
    }

    updateLastState(context.state);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
