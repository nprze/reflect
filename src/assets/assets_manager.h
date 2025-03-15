#pragma once
#include <fstream>
#include "renderer_p/shader/vulkan_shader.h"
namespace rfct {

    class AssetsManager {
    public:
        AssetsManager(std::string path);
        void loadVulkanShader(std::string path, vulkanShader* shaderOut);
        
    private:
        std::string m_Path;

    };

}