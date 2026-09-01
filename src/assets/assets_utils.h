#pragma once

namespace rfct {
	class RfctDevice;
	// assets dir path (varies by platform)
	std::string& GetAssetsPath();
	void SetAssetsPath(const std::string& path);
	// general uses
	bool OpenAssetFile(const std::string& path, std::ifstream* streamOut, std::ios_base::openmode openMode = std::ios::in); // returns true if file is open.
	// command pool uses
	vk::CommandPool& GetAssetsCommandPool(RfctDevice& deviceWrapper);
	void CleanupAssetsCommandPool(vk::Device device);
}