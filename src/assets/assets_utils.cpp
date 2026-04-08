#include "assets_utils.h"
#include <fstream>
#include "renderer_p/renderer.h"

vk::CommandPool assetsCommandPool;
std::string assetsPath;

void rfct::setAssetsPath(const std::string& path) { 
#ifdef WINDOWS_BUILD
    assetsPath = std::string(RFCT_ASSETS_DIR);
#else // android
    assetsPath = path;
#endif // WINDOWS_BUILD 
}

std::string& rfct::getAssetsPath() { return assetsPath; }

bool rfct::openAssetFile(const std::string& path, std::ifstream* streamOut, std::ios_base::openmode openMode) {
    std::string finalPath = getAssetsPath() + "/" + path;
    streamOut->open(finalPath, openMode);
    return streamOut->is_open();
}

vk::CommandPool& rfct::getAssetsCommandPool() {
    if (!assetsCommandPool) {
        assetsCommandPool = renderer::getRen().getDevice().createCommandPool({ {}, renderer::getRen().getDeviceWrapper().getQueueManager().getGraphicsQueueFamilyIndex() });
    }
    return assetsCommandPool;
}

void rfct::cleanupAssetsCommandPool() {
    renderer::getRen().getDevice().destroyCommandPool(assetsCommandPool);
}
