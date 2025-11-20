#pragma once
#include "assets/serialize_structures/user_serialize_data.h"

namespace rfct {
	class userSettings {
	public:
		void loadUserSettings();
		void dumpUserSettings();
		void resetToDefaults();
	private:
		settingsSerializeData seriaizeData;
	};
}