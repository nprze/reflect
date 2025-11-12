#pragma once
#include "miniaudio/miniaudio.h"

namespace rfct {
	class sound {
	public:
		void play();
		~sound();
	private:
		ma_sound internalSound;
		bool isIntitialized = false;
		friend class soundPlayer;
	};
	class soundPlayer {
		soundPlayer();
		~soundPlayer();
		ma_engine m_engine;
	public:
		sound loadSound(const std::string& sound);
	private:
		static soundPlayer instance;
	public:
		static soundPlayer& get() { return instance; }
	};
}