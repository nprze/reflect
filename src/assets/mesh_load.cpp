#include "mesh_load.h"
#include <fstream>
#include "renderer_p/renderer.h"
#include "renderer_p/frame_anim/anim_buffer.h"
#include "assets/assets_utils.h"
#include "serialize_structures/frame_animation_serialize_data.h"

uint32_t basicHash(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

void rfct::uploadVertices(const std::vector<Vertex>& vertices, VulkanBuffer* buffer, vk::DeviceSize offset) {
	RFCT_PROFILE_FUNCTION();
    vk::DeviceSize bufferSize = vertices.size() * sizeof(Vertex);

    VkBufferCreateInfo stagingBufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VmaAllocationCreateInfo stagingAllocInfo = {};
    stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferAllocation;

    if (vmaCreateBuffer(
        RfctRenderer::getRen().getAllocator(),
        &stagingBufferInfo,
        &stagingAllocInfo,
        &stagingBuffer,
        &stagingBufferAllocation,
        nullptr) != VK_SUCCESS)
    {
        RFCT_CRITICAL("Failed to create staging buffer!");
    }

    void* data;
    vmaMapMemory(RfctRenderer::getRen().getAllocator(), stagingBufferAllocation, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vmaUnmapMemory(RfctRenderer::getRen().getAllocator(), stagingBufferAllocation);

    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool = getAssetsCommandPool();
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer;
    RFCT_VULKAN_CHECK(RfctRenderer::getRen().getDevice().allocateCommandBuffers(&allocInfo, &commandBuffer));

    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    commandBuffer.begin(beginInfo);

    vk::BufferCopy copyRegion;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = offset;
    copyRegion.size = bufferSize;

    commandBuffer.copyBuffer(
        vk::Buffer(stagingBuffer),
        buffer->buffer,
        copyRegion);

    commandBuffer.end();

    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    RfctRenderer::getRen().getDeviceWrapper().GetQueue().getPresentQueue().submit(submitInfo);
    RfctRenderer::getRen().getDeviceWrapper().GetQueue().getPresentQueue().waitIdle();

    vmaDestroyBuffer(RfctRenderer::getRen().getAllocator(), stagingBuffer, stagingBufferAllocation);
    RfctRenderer::getRen().getDevice().freeCommandBuffers(getAssetsCommandPool(), commandBuffer);
}

void rfct::loadBuildingBlockMesh(const std::string& path, std::vector<Vertex>* meshOut, const glm::vec3& color, const glm::vec2& size) {
    RFCT_PROFILE_FUNCTION();
    std::ifstream file;
    if (!openAssetFile(path, &file))
        RFCT_CRITICAL("Failed to open mesh file: {}", path);

    std::vector<glm::vec2> coords;
    std::string line;

	Vertex black[6]; // vertices for the black background of the block
    black[0].pos = { 0.f,0.f, 0.f };
    black[1].pos = { size.x,0.f, 0.f };
    black[2].pos = { 0.f,size.y, 0.f };
    black[3].pos = { size.x,size.y, 0.f };
    black[4].pos = { size.x,0.f, 0.f };
    black[5].pos = { 0.f,size.y, 0.f };

    for (uint32_t i = 0; i < 6; ++i) 
    {
        black[i].color = { 0.f, 0.f, 0.f };
        meshOut->push_back(black[i]);
    }
	uint32_t primitiveID = 0;
    while (std::getline(file, line)) 
    {
        std::istringstream iss(line);
        std::vector<float> values;
        double number;

        while (iss >> number) 
        {
            values.push_back(static_cast<float>(number));
        }
        if (values.size() == 2)
        {
            coords.emplace_back(values[0], values[1]);
        }
        else if (values.size() == 1)
        {
            if (coords.size() == 3 || coords.size() == 6)
            {
                constexpr float one255 = 1.f / 255.f;
                glm::vec3 color_fin = glm::vec3(color[0] * values[0] * one255, color[1] * values[0] * one255, color[2] * values[0] * one255);
				float fluctuate = (basicHash(primitiveID) & 0xFFFFFF) / float(0x1000000);
                for (size_t i = 0; i < coords.size(); i++)
                {
                    Vertex vtx{};
                    vtx.pos = glm::vec3(coords[i], 0.0f);
                    vtx.color = color_fin;
					vtx.primitiveFluctuate = fluctuate;
                    meshOut->push_back(vtx);
                }
                coords.clear();

				primitiveID++;
            }
            else RFCT_CRITICAL("Invalid number of coordinates before color line.");
        }
        else RFCT_CRITICAL("Invalid line {} of file {}", line, path);
    }
}

void rfct::loadBackgroundMesh(const std::string& path, std::vector<Vertex>* vertxBufferOut, const glm::vec3& color, const float zMin, const float zMax) {
    RFCT_PROFILE_FUNCTION();
    std::ifstream file;
    if (!openAssetFile(path, &file)) 
    {
        RFCT_CRITICAL("Failed to open mesh file: {}", path);
    }

    std::vector<glm::vec2> coords;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::vector<float> values;

        double number;

        while (iss >> number) {
            values.push_back(static_cast<float>(number));
        }



        if (values.size() == 2) {
            coords.emplace_back(values[0], values[1]);

        }
        if (coords.size() == 4) {
            glm::vec3 color_fin = glm::vec3(color[0] * coords[3].x, color[1] * coords[3].x, color[2] * coords[3].x);

            for (size_t i = 0; i < 3; i++) {
                Vertex vtx{};
                vtx.pos = glm::vec3(coords[i], coords[3].y * (zMax - zMin) + zMin);
                vtx.color = color_fin;
                vertxBufferOut->push_back(vtx);
            }

            coords.clear();
        }
    }
}

void rfct::loadCharacterMesh(const std::string& path, std::vector<Vertex>* meshOut, uint32_t matrixIndexInSSBO) {
    RFCT_PROFILE_FUNCTION();
    std::ifstream file;
    if (!openAssetFile(path, &file)) {
        RFCT_CRITICAL("Failed to open mesh file: {}", path);
    }

    std::vector<glm::vec2> coords;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::vector<float> values;

        double number;

        while (iss >> number) {
            values.push_back(static_cast<float>(number));
        }



        if (values.size() == 2) {
            coords.emplace_back(values[0], values[1]);
        }
        else if (values.size() == 3) {
            glm::vec3 color = { values[0], values[1], values[2] };
            constexpr float one255 = 1.f / 255.f;
            glm::vec3 color_fin = glm::vec3(color[0] * one255, color[1] * one255, color[2] * one255);

            for (size_t i = 0; i < coords.size(); i++) {
                Vertex vtx{};
                vtx.pos = glm::vec3(coords[i], 0.0f);
                vtx.color = color_fin;
                vtx.objectIndex = matrixIndexInSSBO;
                meshOut->push_back(vtx);
            }

            coords.clear();
        }
        else {
            RFCT_CRITICAL("Invalid line {} of file {}", line, path);
        }
    }
}

void rfct::loadAnimation(const std::string& path, frameAnimation* animOut, animationBuffer* loc, uint32_t matrixIndex) {
    RFCT_PROFILE_FUNCTION();
    std::ifstream file;

    if (!openAssetFile(path, &file)) {
        RFCT_CRITICAL("Failed to open frameAnimation file: {}", path);
    }
    std::string line = "";
    uint32_t keyFrameCount = 0;
    float cycleTime = 0;
    uint32_t allTrianglesCount = 0;
    std::string filename;
    std::vector<uint32_t> trianglesCount;
    bool repeatAnimation = true;
    char buffer[256];
    while (std::getline(file, line)) {
        if (line.find("KeyframeCount:") != std::string::npos) {
            RFCT_ASSERT(sscanf(line.c_str(), "KeyframeCount: %d", &keyFrameCount) == 1);
            trianglesCount.reserve(keyFrameCount);
        }
        else if (line.find("CycleTime:") != std::string::npos) {
            RFCT_ASSERT(sscanf(line.c_str(), "CycleTime: %f ", &cycleTime) == 1);
        }
        else if (line.find("AllTrianglesCount:") != std::string::npos) {
            RFCT_ASSERT(sscanf(line.c_str(), "AllTrianglesCount: %d ", &allTrianglesCount) == 1);
        }
        else if (line.find("File:") != std::string::npos) {
            RFCT_ASSERT(sscanf(line.c_str(), "  File: %s", buffer) == 1);
            filename = buffer;
        }
        else if (line.find("Repeat:") != std::string::npos) {
            RFCT_ASSERT(sscanf(line.c_str(), "  Repeat: %s", buffer) == 1);
            if (buffer[0] == '0') {
                repeatAnimation = false;
            }
        }
        else if (line.find("TriangleCount:") != std::string::npos) {
            uint32_t temporaryHolder;
            RFCT_ASSERT(sscanf(line.c_str(), "TriangleCount: %d", &temporaryHolder) == 1);
            trianglesCount.push_back(temporaryHolder);
        }
        else {
            RFCT_CRITICAL("Invalid frameAnimation format. unknown line: {}", line);
        }
    }
    std::vector<Vertex> vertices;
    vertices.reserve(allTrianglesCount * 3);

    std::string finalPath = getAssetsPath() + "/" + path;
    size_t slashPos = finalPath.find_last_of("/\\");
    std::string folderPath;

    if (slashPos != std::string::npos) {
        folderPath = finalPath.substr(0, slashPos + 1);
#ifdef ANDROID_BUILD
        std::string keyword = "files/";
#else
        std::string keyword = "assets/";
#endif // ANDROID_BUILD
        size_t pos = folderPath.find(keyword);
        if (pos != std::string::npos) {
            folderPath = folderPath.substr(pos + keyword.length());
        }
    }

    std::string newPath = folderPath + filename;
    loadCharacterMesh(newPath, &vertices, matrixIndex);

    vulkanBufferLocation location = loc->requestTriangles(allTrianglesCount);
    RFCT_ASSERT(location.buffer); // cannot find buffer to accomodate needs for animation
    uploadVertices(vertices, location.buffer, location.offsetInBytes);

    animOut->buffer = location.buffer;
    animOut->bufferOffsetInBytes = location.offsetInBytes;
    animOut->cycleTime = cycleTime;
    animOut->frameCount = keyFrameCount;
    animOut->shouldBeRepeated = repeatAnimation;
    animOut->trianglesPerFrame = std::move(trianglesCount);
    animOut->timePerFrame = cycleTime / keyFrameCount;
}