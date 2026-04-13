#pragma once
#include "assets/serialize_structures/user_serialize_data.h"

namespace rfct {
	class userSettings {
	public:
		static userSettings& get();
	public:
		void loadUserSettings();
		void dumpUserSettings();
		void applySettings();
	public:
		settingsSerializeData seriaizeData;
	};
}