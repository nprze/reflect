#pragma once


namespace rfct {
	struct settingsSerializeData {
		float masterVoicePercentage = 100;
		float backgroundVoicePercentage = 100;
		float effectsVoicePercentage = 100;
		uint32_t windowedMode = 0; // 0 - borderless, 1 - fullscreen, 2 - windowes
		uint32_t resolutionWidth = 1920;
		uint32_t resolutionHeight  = 1080;
	};
}