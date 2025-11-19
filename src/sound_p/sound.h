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
		soundPlayer();
		~soundPlayer();
		ma_engine m_engine;
		arenaAllocation m_soundMemory;
	public:
		sound loadSound(const std::string& soundPath);
		void playSound(sound& sound);
		void deleteSound(sound& sound);
	};

	class soundManager { // manages sound effect sounds
	private:
		static soundPlayer instance;
	public:
		static soundPlayer& get() { return instance; }
	public:
		sound background;
		sound swoosh;
	public:
		void loadSounds();
		void unloadSounds();
	};
}