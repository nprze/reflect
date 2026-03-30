#pragma once
#include "assets/serialize_structures/user_serialize_data.h"

namespace rfct {
	class userSettings {
		static userSettings instance;
	public:
		static userSettings& get() { return instance; }
		void loadUserSettings();
		void dumpUserSettings();
		void resetToDefaults();
		settingsSerializeData seriaizeData;
	private:
	};
}