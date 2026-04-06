#include "app.h"
#include "world_p/world.h"
#include "ui_p/ui.h"
#include "world_p/progress/user_progress.h"
#include <thread>
#include "job_system_p/job_system.h"
#include "world_p/player/player_animations.h"
#include "sound_p/sound.h"

bool rfct::reflectApplication::isAppMinimised;

rfct::reflectApplication::reflectApplication(RFCT_APP_ARGS)
    : m_Renderer(RFCT_RENDERER_ARGUMENTS_VAR)
{
	input::getInput().init();
	userSettings::get().loadUserSettings();
	isAppMinimised = false;
    loadGameSystems();
	defineUI();
#ifdef WINDOWS_BUILD
    update();
	renderer::getRen().getWindow().show();
	while (renderer::getRen().getWindow().pollAndParseEvents())
        update();
#endif
}

rfct::reflectApplication::~reflectApplication()
{
    renderer::getRen().getDevice().waitIdle();
    cleanGameSystems();
}

void rfct::reflectApplication::updateWindow(RFCT_APP_ARGS)
{
    m_Renderer.updateWindow(RFCT_NATIVE_WINDOW_ANDROID_VAR);
};

void rfct::reflectApplication::update() {
    using clock = std::chrono::steady_clock;

	static float globalTime = 0.f;
    static auto previousTime = clock::now();
    auto frameStart = clock::now();

    std::chrono::duration<float> deltaTime = frameStart - previousTime;
    previousTime = frameStart;

	globalTime += deltaTime.count();

    currentFrame = (currentFrame + 1) % RFCT_FRAMES_IN_FLIGHT;
    frameContext context = {
        .dt = deltaTime.count(),
		.globalTime = globalTime,
        .frame = currentFrame,
        .scene = &world::getWorld().getCurrentScene(),
        .state = getState()
    };

    static float accumulator = 0.f;
    accumulator += context.dt;
    uint64_t timesToUpdate = static_cast<uint64_t>(std::floor(accumulator / fixedDeltaTime));
    accumulator -= timesToUpdate * fixedDeltaTime;

    input::getInput().pollAndParseEvents(&context);

    if (!isAppMinimised) {
        fixedUpdate(context, timesToUpdate);

        jobSystem::get().KickJob([&]() {
            RFCT_PROFILE_SCOPE("ui draw");
            drawUI(&context);
			}, context.wholeUpdateTracker);
#ifdef ANDROID_BUILD
        jobSystem::get().KickJob([&]() {
            RFCT_PROFILE_SCOPE("android UI update");
            input::getInput().drawButtons();
            }, context.wholeUpdateTracker);
#endif
        jobSystem::get().KickJob([&]() {
            RFCT_PROFILE_SCOPE("world visual update");
            world::getWorld().worldVisualUpdate(context);
            }, context.wholeUpdateTracker);
        context.wholeUpdateTracker.waitAll();

        renderer::getRen().render(context);

		// check scene switch
        if (world::getWorld().switchingScenes) {
            world::getWorld().switchScenes(context);
            world::getWorld().switchingScenes = false;
		}
    }

    updateLastState(context.state);
}

void rfct::reflectApplication::fixedUpdate(frameContext& ctx, uint64_t times)
{
    world::getWorld().worldFixedUpdate(ctx, times);
}

void rfct::reflectApplication::loadGameSystems()
{
    auto jobs = std::make_shared<jobTracker>();
    jobSystem::get().KickJob([&]() {
        RFCT_PROFILE_SCOPE("sound load");
        soundPlayer::get().initSoundPlayer();
        soundManager::get().loadSounds();
        }, *jobs);
    jobSystem::get().KickJob([&]() {
        RFCT_PROFILE_SCOPE("animation load");
        playerAnimations::get().loadAnimations();
        }, *jobs);
    jobSystem::get().KickJob([&]() {
        RFCT_PROFILE_SCOPE("init gameplay systems");
        objectSystems::get().init();
        }, *jobs);
    jobs->waitAll();
    world::getWorld().initWorld("world/world.txt");
    play(soundManager::get().background);
}

void rfct::reflectApplication::cleanGameSystems()
{
    world::getWorld().cleanWorld();
    objectSystems::get().cleanup();
    playerAnimations::get().unloadAnimations();
    soundManager::get().unloadSounds();
    soundPlayer::get().cleanupSoundPlayer();
}
