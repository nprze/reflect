#include "sound.h"
#include "assets/assets_manager.h"

rfct::soundPlayer rfct::soundPlayer::instance;
rfct::soundManager rfct::soundManager::instance;

void rfct::soundPlayer::initSoundPlayer()
{
    m_soundMemory = createArena(sizeof(ma_sound) * 10);
    ma_result result;
    result = ma_engine_init(NULL, &m_engine);
    if (result != MA_SUCCESS) {
        RFCT_CRITICAL("Failed to init audio engine");
    }
}

void rfct::soundPlayer::cleanupSoundPlayer()
{
    ma_engine_uninit(&m_engine);
    deleteArena(&m_soundMemory);
}

rfct::sound rfct::soundPlayer::loadSound(const std::string& soundPath)
{
    sound soundData;
    soundData.memory = (ma_sound*)m_soundMemory.allocMemoryArena(sizeof(ma_sound));
    ma_result result;
    result = ma_sound_init_from_file(&m_engine, soundPath.c_str(), 0, NULL, NULL, soundData.memory);
    if (result != MA_SUCCESS) {
        RFCT_ERROR("Failed to load sound: {}", soundPath.c_str());
    }
    return soundData;
}

void rfct::soundPlayer::playSound(sound& sound)
{
    ma_sound_start(sound.memory);
}

void rfct::soundPlayer::deleteSound(sound& sound)
{
    ma_sound_uninit(sound.memory);
}

void rfct::soundManager::loadSounds()
{
    background = soundPlayer::get().loadSound(AssetsManager::get().getPath() + "/" + "sound/sample-background.wav");
    swoosh = soundPlayer::get().loadSound(AssetsManager::get().getPath() + "/" + "sound/swoosh.mp3");
}

void rfct::soundManager::unloadSounds()
{
    soundPlayer::get().deleteSound(background);
    soundPlayer::get().deleteSound(swoosh);
}
