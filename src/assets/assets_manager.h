#pragma once
#include <fstream>
#include "renderer_p/shader/vulkan_shader.h"
#include "renderer_p\image\image.h"
namespace rfct {

    class AssetsManager {
    public:
        AssetsManager(std::string path);
        ~AssetsManager();
        void loadVulkanShader(std::string path, vulkanShader* shaderOut);
		void loadImage(const std::string& path, image* imageOut);
        
    private:
        std::string m_Path;
    };

}