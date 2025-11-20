#include "user_progress.h"
#include "assets/assets_utils.h"
#include <fstream>
#include <iostream>

void rfct::userSettings::loadUserSettings()
{
    std::string finalPath = getAssetsPath() + "player_progress/settings.txt";
    std::ifstream file(finalPath);

    if (!file.is_open()) {
        RFCT_INFO("Failed to open frameAnimation file: {}", finalPath);
    }

    std::string line = "";
    while (std::getline(file, line)) {
    }
}

void rfct::userSettings::dumpUserSettings()
{
}

void rfct::userSettings::resetToDefaults()
{
}