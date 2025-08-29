#pragma once
#include "scene_serialize_data.h"
namespace rfct {
	struct sceneLoader {
		void loadScene(const std::string& path, sceneSerializedData* out);
	};
}