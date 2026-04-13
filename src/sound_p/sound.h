#pragma once
#include "utils/alloc.h"
#include "miniaudio/miniaudio.h"

namespace rfct {
	struct sound {
		ma_sound* memory;
	};
	class soundPlayer { // just plays sounds
	public:
		static soundPlayer& get();
	private:
		ma_engine m_engine;
		arenaAllocation m_soundMemory;
	public:
		void initSoundPlayer();
		void cleanupSoundPlayer();
		sound loadSound(const std::string& soundPath);
		void playSound(sound& sound);
		void deleteSound(sound& sound);
		void applyVolumeSettings();
	};
	class soundManager { // manages sound effect sounds
	public:
		static soundManager& get();
	public:
		sound background;
		sound swoosh;
	public:
		void loadSounds();
		void unloadSounds();
	};
	inline void play(sound& sound) { soundPlayer::get().playSound(sound); }
}