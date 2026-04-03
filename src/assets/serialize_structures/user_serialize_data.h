#pragma once

namespace rfct {
	struct settingsSerializeData {
		uint32_t masterVoicePercentage = 100;
		uint32_t backgroundVoicePercentage = 100;
		uint32_t effectsVoicePercentage = 100;
		uint32_t windowedMode = 0; // 0 - borderless, 1 - fullscreen, 2 - windowes
		uint32_t resolutionWidth = 1920;
		uint32_t resolutionHeight  = 1080;
		uint32_t isDeveloper = 0;
	};
}