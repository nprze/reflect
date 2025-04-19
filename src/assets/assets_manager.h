#pragma once
namespace rfct {
    class vulkanShader;
    class image;
    class font;
    struct mesh;

    // this class exist because the path for assets waries by platform.
    // in java android app code, the data from assets folder is copied to the app local data folder where the data can be freely read/modified.
    class AssetsManager {
    public:
        void loadVulkanShader(std::string path, vulkanShader* shaderOut);
		void loadImage(const std::string& path, image* imageOut);
        void loadGlyphs(const std::string& path, font* fontOut);
        void loadMesh(const std::string& path, mesh* vertxBufferOut);
    private:
        std::string m_Path;

        // should be constructed only from app class
        AssetsManager(std::string path);
        ~AssetsManager();
        friend class reflectApplication;
    };
}