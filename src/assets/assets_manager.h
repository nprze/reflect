#pragma once
#include <fstream>
namespace rfct {
    class vulkanShader;
    class image;
    class font;
    struct mesh;

    class AssetsManager {
    public:
        AssetsManager(std::string path);
        ~AssetsManager();
        void loadVulkanShader(std::string path, vulkanShader* shaderOut);
		void loadImage(const std::string& path, image* imageOut);
        void loadGlyphs(const std::string& path, font* fontOut);
        void loadMesh(const std::string& path, mesh* vertxBufferOut);
        
    private:
        std::string m_Path;
    };

}