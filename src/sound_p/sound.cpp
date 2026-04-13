#include "sound.h"
#include "assets/assets_utils.h"
#include "world_p/progress/user_progress.h"

rfct::soundPlayer soundPlayerInstance;
rfct::soundManager soundManagerInstance;

rfct::soundManager& rfct::soundManager::get() { return soundManagerInstance; }
rfct::soundPlayer& rfct::soundPlayer::get() { return soundPlayerInstance; }

void rfct::soundPlayer::initSoundPlayer() {
    RFCT_PROFILE_FUNCTION();
    m_soundMemory = createArena(sizeof(ma_sound) * 10);
    ma_result result;
    result = ma_engine_init(NULL, &m_engine);
    if (result != MA_SUCCESS) RFCT_CRITICAL("Failed to init audio engine");
}

void rfct::soundPlayer::cleanupSoundPlayer() {
    RFCT_PROFILE_FUNCTION();
    ma_engine_uninit(&m_engine);
    deleteArena(&m_soundMemory);
}

rfct::sound rfct::soundPlayer::loadSound(const std::string& soundPath) {
    RFCT_PROFILE_FUNCTION();
    sound soundData;
    soundData.memory = (ma_sound*)m_soundMemory.allocMemoryArena(sizeof(ma_sound));
    ma_result result;
    result = ma_sound_init_from_file(&m_engine, soundPath.c_str(), 0, NULL, NULL, soundData.memory);
    if (result != MA_SUCCESS) RFCT_ERROR("Failed to load sound: {}", soundPath.c_str());
    return soundData;
}

void rfct::soundPlayer::playSound(sound& sound) {
    ma_sound_start(sound.memory);
}

void rfct::soundPlayer::deleteSound(sound& sound) {
    ma_sound_uninit(sound.memory);
}

void rfct::soundPlayer::applyVolumeSettings() {
    RFCT_PROFILE_FUNCTION();
    settingsSerializeData& settings = userSettings::get().seriaizeData;
	constexpr float oneOverHundred = 1.f / 100.f;
    ma_engine_set_volume(&m_engine, settings.masterVoicePercentage * oneOverHundred);
}

void rfct::soundManager::loadSounds() {
    RFCT_PROFILE_FUNCTION();
    background = soundPlayer::get().loadSound(getAssetsPath() + "/sound/sample-background.wav");
    swoosh = soundPlayer::get().loadSound(getAssetsPath() + "/sound/swoosh.mp3");
}

void rfct::soundManager::unloadSounds() {
    soundPlayer::get().deleteSound(background);
    soundPlayer::get().deleteSound(swoosh);
}
