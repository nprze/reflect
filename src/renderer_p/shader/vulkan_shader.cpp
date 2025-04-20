#include "vulkan_shader.h"

#include "assets/assets_manager.h"

namespace rfct {
    vulkanShader::vulkanShader(const std::string& spirvFilePath)
    {
        AssetsManager::get().loadVulkanShader(spirvFilePath, this);
    }

}