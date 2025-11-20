#include "game.h"
#include "world_p/world.h"
#include "world_p/player/player_animations.h"
#include "sound_p/sound.h"
#include "job_system_p/job_system.h"

void rfct::initGame()
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
		}, * jobs);
	jobs->waitAll();
	world::getWorld().initWorld("");
	play(soundManager::get().background);
}

void rfct::updateGame(frameContext& ContextArg)
{
	world::getWorld().onUpdate(ContextArg);
}

void rfct::cleanupGame()
{
	world::getWorld().cleanWorld();
	objectSystems::get().cleanup();
	playerAnimations::get().unloadAnimations();
	soundManager::get().unloadSounds();
	soundPlayer::get().cleanupSoundPlayer();
}
