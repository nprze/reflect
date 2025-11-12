#include "sound.h"
rfct::soundPlayer rfct::soundPlayer::instance;
rfct::soundPlayer::soundPlayer()
{
    ma_engine_config config = ma_engine_config_init();

    if (ma_engine_init(&config, &m_engine) != MA_SUCCESS) {
        RFCT_CRITICAL("Failed to init engine");
    }
}

rfct::soundPlayer::~soundPlayer()
{
    ma_engine_uninit(&m_engine);
}

rfct::sound rfct::soundPlayer::loadSound(const std::string& path)
{
	sound s;

    RFCT_ASSERT(ma_sound_init_from_file(&m_engine, path.c_str(), 0, nullptr, nullptr, &s.internalSound) == MA_SUCCESS);

    s.isIntitialized = true;

    return s;
}

void rfct::sound::play()
{
    ma_sound_start(&internalSound);
}

rfct::sound::~sound()
{
    if (isIntitialized) {
        ma_sound_uninit(&internalSound);
    }
}
