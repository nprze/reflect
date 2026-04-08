#pragma once

namespace rfct {
	// assets dir path (varies by platform)
	void setAssetsPath(const std::string& path);
	std::string& getAssetsPath();
	// general uses
	bool openAssetFile(const std::string& path, std::ifstream* streamOut, std::ios_base::openmode openMode = std::ios::in); // returns true if file is open.
	// command pool uses
	vk::CommandPool& getAssetsCommandPool();
	void cleanupAssetsCommandPool();
}