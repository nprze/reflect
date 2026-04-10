#pragma once
#include "assets/serialize_structures/user_serialize_data.h"

namespace rfct {
	class userSettings {
	public:
		static userSettings& get();
		void loadUserSettings();
		void dumpUserSettings();
	public:
		settingsSerializeData seriaizeData;
	};
}