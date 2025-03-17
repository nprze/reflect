#include "assets_manager.h"
#include "app.h"
namespace rfct {
    AssetsManager::AssetsManager(std::string path){
        if (path.length()!=0) {
            m_Path = path;
            return;
        }else{
            m_Path = std::string(RFCT_ASSETS_DIR);
        }
    }
    AssetsManager::~AssetsManager()
    {
    }
    void AssetsManager::loadVulkanShader(std::string path, vulkanShader* shaderOut){
        std::string finalPath = m_Path+"/"+path;
        std::ifstream file(finalPath,std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            RFCT_CRITICAL("Failed to open shader file: {}", finalPath);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        file.seekg(0);

        std::vector<char> buffer(fileSize);
        file.read(buffer.data(), fileSize);
        file.close();

        vk::ShaderModuleCreateInfo createInfo = {};
        createInfo.setCodeSize(buffer.size());
        createInfo.setPCode(reinterpret_cast<const uint32_t*>(buffer.data()));

        shaderOut->m_shaderModule = renderer::getRen().getDevice().createShaderModuleUnique(createInfo);
    }

}