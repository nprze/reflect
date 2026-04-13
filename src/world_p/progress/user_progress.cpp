#include "user_progress.h"
#include <fstream>
#include <iostream>
#include "assets/assets_utils.h"
#include "sound_p/sound.h"

rfct::userSettings instance;

rfct::userSettings& rfct::userSettings::get() { return instance; }

#define SETTINGS_GET_STRING(label, out)                                                                                 \
    { std::getline(file, line);                                                                                     \
    out = line.substr(sizeof(label) + 2);                                                                           \
    while (!out.empty() && out.back() == '\r') out.pop_back(); }                                                                                                          

#define SETTINGS_GET_INT(label, out) std::getline(file, line); RFCT_ASSERT(sscanf(line.c_str(), label " %d", &out) == 1);

#define SETTINGS_GET_FLOAT(label, out) std::getline(file, line); RFCT_ASSERT(sscanf(line.c_str(), label " %f", &out) == 1);                                                                                  

#define SETTINGS_SET_ANY(label, value) \
    file << label << " " << value << "\n";

void rfct::userSettings::loadUserSettings() {
    RFCT_PROFILE_FUNCTION();
    std::string finalPath = getAssetsPath() + "/player_progress/settings.txt";
    std::ifstream file(finalPath);

    if (!file.is_open()) {
		// create default file
		dumpUserSettings();
		file.open(finalPath);
        if (!file.is_open()) {
            RFCT_CRITICAL("Failed to open user settings file: {}", finalPath);
        }
    }

    std::string line = "";
    SETTINGS_GET_INT("masterVoicePercentage:", seriaizeData.masterVoicePercentage);
    SETTINGS_GET_INT("backgroundVoicePercentage:", seriaizeData.backgroundVoicePercentage);
    SETTINGS_GET_INT("effectsVoicePercentage:", seriaizeData.effectsVoicePercentage);
    SETTINGS_GET_INT("windowedMode:", seriaizeData.windowedMode);
    SETTINGS_GET_INT("resolutionWidth:", seriaizeData.resolutionWidth);
 	SETTINGS_GET_INT("resolutionHeight:", seriaizeData.resolutionHeight);
 	SETTINGS_GET_INT("isDeveloper:", seriaizeData.isDeveloper);
}

void rfct::userSettings::dumpUserSettings() {
    RFCT_PROFILE_FUNCTION();
    std::string finalPath = getAssetsPath() + "/player_progress/settings.txt";
    std::ofstream file(finalPath, std::ios::out | std::ios::trunc);
    if (!file) {
		RFCT_CRITICAL("Failed to open user settings file for writing: {}", finalPath);
    }
    SETTINGS_SET_ANY("masterVoicePercentage:", seriaizeData.masterVoicePercentage);
    SETTINGS_SET_ANY("backgroundVoicePercentage:", seriaizeData.backgroundVoicePercentage);
    SETTINGS_SET_ANY("effectsVoicePercentage:", seriaizeData.effectsVoicePercentage);
    SETTINGS_SET_ANY("windowedMode:", seriaizeData.windowedMode);
    SETTINGS_SET_ANY("resolutionWidth:", seriaizeData.resolutionWidth);
    SETTINGS_SET_ANY("resolutionHeight:", seriaizeData.resolutionHeight);
    SETTINGS_SET_ANY("isDeveloper:", seriaizeData.isDeveloper);
}

void rfct::userSettings::applySettings() {
    RFCT_PROFILE_FUNCTION();
	soundPlayer::get().applyVolumeSettings();
}
