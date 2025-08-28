#pragma once
#include "scene_serialize_data.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"
#include "dialogue_serialize_data.h"
#include "button_image_serialize_data.h"


namespace rfct {
    class vulkanShader;
    class image;
    class font;
    struct buildingBlockMesh;
    struct frameAnimation;
    class VulkanBuffer;

    // this class exist because the path for assets waries by platform.
    // in java android app code, the data from assets folder is copied to the app local data folder where the data can be freely read/modified.
    class AssetsManager {
        static AssetsManager instance;
    public:
        static AssetsManager& get() { return instance; }
        void init(std::string path);
        void cleanup();
        void loadVulkanShader(std::string path, vulkanShader* shaderOut);
		void loadImage(const std::string& path, image* imageOut);
        void loadGlyphs(const std::string& path, font* fontOut);
        void loadBuildingBlockMesh(const std::string& path, std::vector<Vertex>* vertxBufferOut, const glm::vec3& color, const glm::vec2& size);
        void loadBackgroundMesh(const std::string& path, std::vector<Vertex>* vertxBufferOut, const glm::vec3& color, const float zMin, const float zMax);
        void loadCharacterMesh(const std::string& path, std::vector<Vertex>* meshOut);
        void loadScene(const std::string& path, sceneSerializedData* sceneSerializedDataOut);
        void loadDialogue(const std::string& path, dialogueSerializeData* dialogueSerializedDataOut);
        void loadDialogueSpriteSheet(const std::string& path, dialogueSpritesheetSerializeData* dialogueSpritesheetSerializedDataOut);

        void loadButtonImage(const std::string& path, buttonImageSerializeData* buttonImageSerializedDataOut);

        void uploadVertices(const std::vector<Vertex>& vertices, VulkanBuffer* buffer, vk::DeviceSize offset); // helper function

        frameAnimation loadAnimation(const std::string& path);
        void createDummyImage(image* imageOut);
        vk::CommandPool& getCommandPool();
    private:
        vk::CommandPool m_AssetsCommandPool;
        std::string m_Path;

        AssetsManager() = default;
        ~AssetsManager() = default;
    };
}