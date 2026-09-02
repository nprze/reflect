#include "vulkan_shader.h"
#include <fstream>
#include "assets/assets_utils.h"
#include "renderer_p/renderer.h"

namespace rfct {
    vulkanShader::vulkanShader(const std::string& spirvFilePath) {
        RFCT_PROFILE_FUNCTION();
        std::ifstream file;
        if (!OpenAssetFile(spirvFilePath, &file, std::ios::binary | std::ios::ate)) {
            RFCT_CRITICAL("Failed to open shader file: {}", spirvFilePath);
        }
        size_t fileSize = static_cast<size_t>(file.tellg());
        file.seekg(0);

        std::vector<char> buffer(fileSize);
        file.read(buffer.data(), fileSize);
        file.close();

        vk::ShaderModuleCreateInfo createInfo = {};
        createInfo.setCodeSize(buffer.size());
        createInfo.setPCode(reinterpret_cast<const uint32_t*>(buffer.data()));

        m_shaderModule = GetRen().GetDevice().createShaderModuleUnique(createInfo).value;
    }
}