#include "asset_manager.h"

rfct::RfctAssetManager& rfct::GetAssetManager() {
	static rfct::RfctAssetManager g_assetManager;
	return g_assetManager;
}

rfct::RfctShader* rfct::RfctAssetManager::GetOrLoadShader(vk::Device device, const std::string& path) {
	auto [it, inserted] = m_shaders.try_emplace(path, device, path);
	return &it->second;
}
