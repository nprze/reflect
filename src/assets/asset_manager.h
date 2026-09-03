#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "renderer_p/components/render_objects.h"

namespace rfct {
	// All paths passed to asset manager functions must be relative to the assets/ directory.
	class RfctAssetManager {
	public:
		RfctShader* GetOrLoadShader(vk::Device device, const std::string& path);
	private:
		std::map<std::string, RfctShader> m_shaders;
	};
	RfctAssetManager& GetAssetManager();
}