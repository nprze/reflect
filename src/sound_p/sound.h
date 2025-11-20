#pragma once
#include "miniaudio/miniaudio.h"

namespace rfct {
	struct sound {
		ma_sound* memory;
	};

	class soundPlayer { // just plays sounds
	private:
		static soundPlayer instance;
	public:
		static soundPlayer& get() { return instance; }
	private:
		ma_engine m_engine;
		arenaAllocation m_soundMemory;
	public:
		void initSoundPlayer();
		void cleanupSoundPlayer();
		sound loadSound(const std::string& soundPath);
		void playSound(sound& sound);
		void deleteSound(sound& sound);
	};

	inline void play(sound& sound) { soundPlayer::get().playSound(sound); }

	class soundManager { // manages sound effect sounds
	private:
		static soundManager instance;
	public:
		static soundManager& get() { return instance; }
	public:
		sound background;
		sound swoosh;
	public:
		void loadSounds();
		void unloadSounds();
	};
}