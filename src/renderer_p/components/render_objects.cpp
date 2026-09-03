#include "render_objects.h"
#include "assets/assets_utils.h"
#include <fstream>

rfct::RfctShader::RfctShader(vk::Device device, const std::string& spirvFilePath) {
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

    m_shaderModule = device.createShaderModuleUnique(createInfo).value;
}
