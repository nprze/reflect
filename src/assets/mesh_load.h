#pragma once
#include "renderer_p/rasterizer_pipeline/vertex.h"

namespace rfct {
    class VulkanBuffer;
    class Vertex;
    class frameAnimation;
    class animationBuffer;

    void uploadVertices(const std::vector<Vertex>& vertices, VulkanBuffer* buffer, vk::DeviceSize offset); // helper function
    void loadAnimation(const std::string& path, frameAnimation* animOut, animationBuffer* location, uint32_t matrixIndex = 1);
    void loadBuildingBlockMesh(const std::string& path, std::vector<Vertex>* vertxBufferOut, const glm::vec3& color, const glm::vec2& size);
    void loadBackgroundMesh(const std::string& path, std::vector<Vertex>* vertxBufferOut, const glm::vec3& color, const float zMin, const float zMax);
    void loadCharacterMesh(const std::string& path, std::vector<Vertex>* meshOut, uint32_t matrixIndexInSSBO);
}