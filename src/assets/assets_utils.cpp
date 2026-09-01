#include "assets_utils.h"
#include "renderer_p/components/renderer_components.h"
#include <fstream>

vk::CommandPool assetsCommandPool;
std::string assetsPath;

void rfct::SetAssetsPath(const std::string& path) { 
#ifdef WINDOWS_BUILD
    assetsPath = std::string(RFCT_ASSETS_DIR);
#else // android
    assetsPath = path;
#endif // WINDOWS_BUILD 
}

std::string& rfct::GetAssetsPath() { return assetsPath; }

bool rfct::OpenAssetFile(const std::string& path, std::ifstream* streamOut, std::ios_base::openmode openMode) {
    std::string finalPath = GetAssetsPath() + "/" + path;
    streamOut->open(finalPath, openMode);
    return streamOut->is_open();
}

vk::CommandPool& rfct::GetAssetsCommandPool(RfctDevice& deviceWrapper) {
    if (!assetsCommandPool) {
        auto createCommandPoolResult = deviceWrapper.GetDevice().createCommandPool({ {}, deviceWrapper.GetQueue().getGraphicsQueueFamilyIndex() });
        RFCT_VULKAN_CHECK(createCommandPoolResult.result);
        assetsCommandPool = createCommandPoolResult.value;
    }
    return assetsCommandPool;
}

void rfct::CleanupAssetsCommandPool(vk::Device device) {
    device.destroyCommandPool(assetsCommandPool);
}
