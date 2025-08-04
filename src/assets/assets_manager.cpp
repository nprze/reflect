#include "assets_manager.h"
#include "app.h"
#include "stb_image/stb_image.h"
#include <fstream>

#include "renderer_p/shader/vulkan_shader.h"
#include "renderer_p/image/image.h"
#include "renderer_p/UI/font/font.h"
#include "renderer_p/mesh/mesh.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"
#include "assets/frame_animation.h"
#include "world_p/player/player_animations.h"
#include "renderer_p/buffer/vulkan_buffer.h"

namespace rfct {
    void AssetsManager::uploadVertices(const std::vector<Vertex>& vertices, VulkanBuffer* buffer, vk::DeviceSize offset)
    {
        vk::DeviceSize bufferSize = vertices.size() * sizeof(Vertex);

        // 1. Create a staging buffer
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
            renderer::getRen().getAllocator(),
            &stagingBufferInfo,
            &stagingAllocInfo,
            &stagingBuffer,
            &stagingBufferAllocation,
            nullptr) != VK_SUCCESS)
        {
            RFCT_CRITICAL("Failed to create staging buffer!");
        }

        // 2. Copy data to the staging buffer
        void* data;
        vmaMapMemory(renderer::getRen().getAllocator(), stagingBufferAllocation, &data);
        memcpy(data, vertices.data(), (size_t)bufferSize);
        vmaUnmapMemory(renderer::getRen().getAllocator(), stagingBufferAllocation);

        // 3. Record command buffer to copy from staging to vertex buffer (with offset)
        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.commandPool = m_AssetsCommandPool;
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandBufferCount = 1;

        vk::CommandBuffer commandBuffer;
        RFCT_VULKAN_CHECK(renderer::getRen().getDevice().allocateCommandBuffers(&allocInfo, &commandBuffer));

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

        // 4. Submit the command buffer
        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        renderer::getRen().getDeviceWrapper().getQueueManager().getPresentQueue().submit(submitInfo);
        renderer::getRen().getDeviceWrapper().getQueueManager().getPresentQueue().waitIdle();

        // 5. Cleanup staging buffer
        vmaDestroyBuffer(renderer::getRen().getAllocator(), stagingBuffer, stagingBufferAllocation);
        renderer::getRen().getDevice().freeCommandBuffers(m_AssetsCommandPool, commandBuffer);
    }


    AssetsManager AssetsManager::instance;

    void AssetsManager::init(std::string path)
    {
#ifdef WINDOWS_BUILD
        m_Path = std::string(RFCT_ASSETS_DIR);
#else
        // android
        m_Path = path;
#endif // WINDOWS_BUILD
    }

    void AssetsManager::cleanup()
    {
        renderer::getRen().getDevice().destroyCommandPool(m_AssetsCommandPool);
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

    void AssetsManager::loadImage(const std::string& path, image* imageOut)
	{
		std::string finalPath = m_Path + "/" + path;
        int width, height, channels;
        stbi_uc* pixels = stbi_load(finalPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (!pixels) {
            RFCT_CRITICAL("Failed to load image: {}", finalPath);
        }

        vk::DeviceSize imageSize = width * height * 4;
        imageOut->width = width;
        imageOut->height = height;

        // Create staging buffer
        vk::BufferCreateInfo bufferInfo{ {}, imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive };
        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        vk::Buffer stagingBuffer;
        VmaAllocation stagingBufferAllocation;
        if (vmaCreateBuffer(renderer::getRen().getAllocator(), reinterpret_cast<const VkBufferCreateInfo*>(&bufferInfo), &allocCreateInfo,
            reinterpret_cast<VkBuffer*>(&stagingBuffer), &stagingBufferAllocation, nullptr) != VK_SUCCESS) {
            RFCT_CRITICAL("Failed to create staging buffer");
        }

        // Copy pixel data to the buffer
        void* data;
        vmaMapMemory(renderer::getRen().getAllocator(), stagingBufferAllocation, &data);
        std::memcpy(data, pixels, static_cast<size_t>(imageSize));
        vmaUnmapMemory(renderer::getRen().getAllocator(), stagingBufferAllocation);
        stbi_image_free(pixels);

        // Create Vulkan image
        vk::ImageCreateInfo imageInfo({}, vk::ImageType::e2D, vk::Format::eR8G8B8A8Unorm,
            { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 }, 1, 1,
            vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
            vk::SharingMode::eExclusive);

        VmaAllocationCreateInfo imageAllocInfo{};
        imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        if (vmaCreateImage(renderer::getRen().getAllocator(), reinterpret_cast<const VkImageCreateInfo*>(&imageInfo), &imageAllocInfo,
            reinterpret_cast<VkImage*>(&imageOut->m_image), &imageOut->m_allocation, nullptr) != VK_SUCCESS) {
            RFCT_CRITICAL("Failed to create Vulkan image");
        }

        // Allocate command buffer
        if (!m_AssetsCommandPool){
            m_AssetsCommandPool = renderer::getRen().getDevice().createCommandPool({ {}, renderer::getRen().getDeviceWrapper().getQueueManager().getGraphicsQueueFamilyIndex() });
        }
        vk::CommandBufferAllocateInfo allocInfo(m_AssetsCommandPool, vk::CommandBufferLevel::ePrimary, 1);
        vk::CommandBuffer commandBuffer = renderer::getRen().getDevice().allocateCommandBuffers(allocInfo)[0];

        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        commandBuffer.begin(beginInfo);

        // Transition image layout and copy buffer data
        imageOut->transitionImageLayout(commandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        imageOut->copyBufferToImage(commandBuffer, stagingBuffer);
        imageOut->transitionImageLayout(commandBuffer, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

        commandBuffer.end();

        vk::SubmitInfo submitInfo({}, {}, commandBuffer);
		vk::FenceCreateInfo fenceInfo;
		vk::Fence fence = renderer::getRen().getDevice().createFence(fenceInfo);
        renderer::getRen().getDeviceWrapper().getQueueManager().submitGraphics(submitInfo, fence);
		RFCT_VULKAN_CHECK(renderer::getRen().getDevice().waitForFences(fence, VK_TRUE, UINT64_MAX));

        renderer::getRen().getDevice().freeCommandBuffers(m_AssetsCommandPool, commandBuffer);
        vmaDestroyBuffer(renderer::getRen().getAllocator(), static_cast<VkBuffer>(stagingBuffer), stagingBufferAllocation);
		renderer::getRen().getDevice().destroyFence(fence);

        // Create Image View
        vk::ImageViewCreateInfo viewInfo({}, imageOut->m_image, vk::ImageViewType::e2D, vk::Format::eR8G8B8A8Unorm, {},
            { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
        imageOut->m_imageView = renderer::getRen().getDevice().createImageView(viewInfo);
    }

    void AssetsManager::loadGlyphs(const std::string& path, font* fontOut)
    {
        std::string finalPath = m_Path + "/" + path;
        std::ifstream file(finalPath);
        if (!file.is_open()) {
            RFCT_CRITICAL("Failed to open font data file: {}", finalPath);
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.rfind("char id=", 0) == 0) {
                std::istringstream stream(line);
                std::string key;
                int id = -1;
                glyph g{};
                while (stream >> key) {
                    if (key.find("id=") == 0) id = std::stoi(key.substr(3));
                    else if (key.find("x=") == 0) g.x = std::stof(key.substr(2));
                    else if (key.find("y=") == 0) g.y = std::stof(key.substr(2));
                    else if (key.find("width=") == 0) g.width = std::stof(key.substr(6));
                    else if (key.find("height=") == 0) g.height = std::stof(key.substr(7));
                    else if (key.find("xoffset=") == 0) g.xoffset = std::stof(key.substr(8));
                    else if (key.find("yoffset=") == 0) g.yoffset = std::stof(key.substr(8));
                    else if (key.find("xadvance=") == 0) g.xadvance = std::stof(key.substr(9));
                }
                fontOut->glyphMap[static_cast<char>(id)] = g;
            }
        }
    }

    void AssetsManager::loadBuildingBlockMesh(const std::string& path, std::vector<Vertex>* meshOut, const glm::vec3& color, const glm::vec2& size)
    {
        std::string finalPath = m_Path + "/" + path;
        std::ifstream file(finalPath);

        if (!file.is_open()) {
            RFCT_CRITICAL("Failed to open mesh file: {}", finalPath);
        }

        std::vector<glm::vec2> coords;
        std::string line;

        Vertex black[6];
        black[0].pos = { 0.f,0.f, 0.f };
        black[1].pos = { size.x,0.f, 0.f };
        black[2].pos = { 0.f,size.y, 0.f };
        black[3].pos = { size.x,size.y, 0.f };
        black[4].pos = { size.x,0.f, 0.f };
        black[5].pos = { 0.f,size.y, 0.f };

        for (uint32_t i = 0; i < 6; ++i) {
            black[i].color = { 0.f, 0.f, 0.f };
            meshOut->push_back(black[i]);
        }
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
            else if (values.size() == 1) {
                if (coords.size() == 3 || coords.size() == 6) {
                    constexpr float one255 = 1.f / 255.f;
                    glm::vec3 color_fin = glm::vec3(color[0] * values[0] * one255, color[1] * values[0] * one255, color[2] * values[0] * one255);

                    for (size_t i = 0; i < coords.size(); i++) {
                        Vertex vtx{};
                        vtx.pos = glm::vec3(coords[i], 0.0f);
                        vtx.color = color_fin;
                        meshOut->push_back(vtx);
                    }

                    coords.clear();
                }
                else {
                    RFCT_CRITICAL("Invalid number of coordinates before color line.");
                }
            }
            else {
                RFCT_CRITICAL("Invalid line {} of file {}", line, finalPath);
            }
        }

    }

    void AssetsManager::loadBackgroundMesh(const std::string& path, std::vector<Vertex>* vertxBufferOut, const glm::vec3& color, const float zMin, const float zMax)
    {

        std::string finalPath = m_Path + "/" + path;
        std::ifstream file(finalPath);

        if (!file.is_open()) {
            RFCT_CRITICAL("Failed to open mesh file: {}", finalPath);
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
            if (coords.size()==4) {
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

    void AssetsManager::loadCharacterMesh(const std::string& path, std::vector<Vertex>* meshOut)
    {
        std::string finalPath = m_Path + "/" + path;
        std::ifstream file(finalPath);

        if (!file.is_open()) {
            RFCT_CRITICAL("Failed to open mesh file: {}", finalPath);
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
                    meshOut->push_back(vtx);
                }

                coords.clear();
            }
            else {
                RFCT_CRITICAL("Invalid line {} of file {}", line, finalPath);
            }
        }
    }

    void AssetsManager::loadScene(const std::string& path, sceneSerializedData* sceneSerializedDataOut)
    {
        // IO only here
        std::string finalPath = m_Path + "/" + path;
        std::ifstream file(finalPath);

        if (!file.is_open()) {
            RFCT_CRITICAL("Failed to open scene file: {}", finalPath);
        }

        std::string line;
        bool rectFullyOk = false;
        rectangle current;
        int minx, miny;
        int maxx, maxy;

        float xin, yin;


        bool vineFullyOk = false;
        vineInfo vn = {};

        while (std::getline(file, line)) {
            if (line.find("Rect:") != std::string::npos) {
                rectFullyOk = false;
                current = {};
                while (!rectFullyOk) {
                    std::getline(file, line); 
                    if (line.find("color:") != std::string::npos) {
                        current.color = line.substr(line.find(":") + 2);
                    }
                    else if (line.find("min:") != std::string::npos) {

                        RFCT_ASSERT(sscanf(line.c_str(), "  min: (%d, %d)", &minx, &miny) == 2);
                        current.min.x = minx;
                        current.min.y = miny;
                    }
                    else if (line.find("max:") != std::string::npos) {
                        RFCT_ASSERT(sscanf(line.c_str(), "  max: (%d, %d)", &maxx, &maxy) == 2);
                        current.max.x = maxx;
                        current.max.y = maxy;
                    }
                    else if (line.find("file:") != std::string::npos) {
                        current.file = line.substr(line.find(":") + 2);
                        if (current.file.back() == '\r') {
                            current.file.pop_back();
                        }
                        rectFullyOk = true;
                    }
                }
                sceneSerializedDataOut->rectangles.push_back(current);
            }
            else if (line.find("Vine:") != std::string::npos) {
                vn = {};
                vineFullyOk = false;
                while (!vineFullyOk) {
                    std::getline(file, line);
                    if (line.find("start:") != std::string::npos) {

                        RFCT_ASSERT(sscanf(line.c_str(), "  start: (%f, %f)", &xin, &yin) == 2);
                        vn.start.x = xin;
                        vn.start.y = yin;
                    }
                    else if (line.find("end:") != std::string::npos) {
                        RFCT_ASSERT(sscanf(line.c_str(), "  end: (%f, %f)", &xin, &yin) == 2);
                        vn.end.x = xin;
                        vn.end.y = yin;
                    }
                    else if (line.find("edges:") != std::string::npos) {
                        RFCT_ASSERT(sscanf(line.c_str(), "  edges: %d", &vn.numEdges) == 1);
                        vineFullyOk = true;
                    }
                }
                sceneSerializedDataOut->vines.push_back(vn);
            }
            else if (line.find("SceneWidth:") != std::string::npos) {
                RFCT_ASSERT(sscanf(line.c_str(), "SceneWidth: %d", &sceneSerializedDataOut->width) == 1);
            }
            else if (line.find("SceneHeight:") != std::string::npos) {
                RFCT_ASSERT(sscanf(line.c_str(), "SceneHeight: %d", &sceneSerializedDataOut->height) == 1);
            }
            else if (line.find("RectCount:") != std::string::npos) {
                int rectCount = 0;
                RFCT_ASSERT(sscanf(line.c_str(), "RectCount: %d", &rectCount) == 1);
                sceneSerializedDataOut->rectangles.reserve(rectCount);
            }
            else if (line.find("VineCount:") != std::string::npos) {
                int vinesCount = 0;
                RFCT_ASSERT(sscanf(line.c_str(), "VineCount: %d", &vinesCount) == 1);
                sceneSerializedDataOut->vines.reserve(vinesCount);
            }
        }
        /*
        // Output parsed rectangles
        for (size_t i = 0; i < sceneSerializedDataOut->rectangles.size(); ++i) {
            const auto& r = sceneSerializedDataOut->rectangles[i];
            RFCT_INFO("Rectangle {}", i);
            RFCT_INFO("  Color: {}", r.color);
            RFCT_INFO("  AABB Min: ({}, {})", r.min.x, r.min.y);
            RFCT_INFO("  AABB Max: ({}, {})", r.max.x, r.max.y);
            RFCT_INFO("  cutoff: {}", r.cutoff);
            RFCT_INFO("  cutoff_top: {}", (r.cutoff & cutoffValues::top) ? "true" : "false");
            RFCT_INFO("  exists file: {}", r.file);
        }*/
    }

    frameAnimation AssetsManager::loadAnimation(const std::string& path)
    {
        std::string finalPath = m_Path + "/" + path;
        std::ifstream file(finalPath);

        if (!file.is_open()) {
            RFCT_CRITICAL("Failed to open frameAnimation file: {}", finalPath);
        }

        std::string line = "";
        uint32_t keyFrameCount = 0;
        float cycleTime = 0;
        uint32_t allTrianglesCount = 0;
        std::string filename;
        std::vector<uint32_t> trianglesCount;
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

        size_t slashPos = finalPath.find_last_of("/\\");
        std::string folderPath;

        if (slashPos != std::string::npos) {
            folderPath = finalPath.substr(0, slashPos + 1);
            //std::string keyword = "assets/";
            std::string keyword = "files/";
            size_t pos = folderPath.find(keyword);
            if (pos != std::string::npos) {
                folderPath = folderPath.substr(pos + keyword.length());
            }
        }

        std::string newPath = folderPath + filename;

        loadCharacterMesh(newPath, &vertices);
        

        vulkanBufferLocation loc = playerAnimations::get().requestVulkanBuffer(allTrianglesCount);

        uploadVertices(vertices, loc.buffer, loc.offsetInBytes);
        

        frameAnimation anim;
        anim.buffer = loc.buffer;
        anim.bufferOffsetInBytes = loc.offsetInBytes;
        anim.cycleTime = cycleTime;
        anim.frameCount = keyFrameCount;
        anim.trianglesPerFrame = std::move(trianglesCount);
        anim.timePerFrame = cycleTime / keyFrameCount;
        return anim;
    }

    void AssetsManager::createDummyImage(image* imageOut)
    {

        uint32_t widthHeight = 1;
        vk::DeviceSize imageSize = 1;

        // Create Vulkan image
        vk::ImageCreateInfo imageInfo({}, vk::ImageType::e2D, vk::Format::eR8G8B8A8Unorm,
            { static_cast<uint32_t>(widthHeight), static_cast<uint32_t>(widthHeight), 1 }, 1, 1,
            vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eSampled,
            vk::SharingMode::eExclusive);

        VmaAllocationCreateInfo imageAllocInfo{};
        imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VkResult res = vmaCreateImage(renderer::getRen().getAllocator(), reinterpret_cast<const VkImageCreateInfo*>(&imageInfo), &imageAllocInfo,
            reinterpret_cast<VkImage*>(&imageOut->m_image), &imageOut->m_allocation, nullptr);
        if (res != VK_SUCCESS) {
            RFCT_CRITICAL("Failed to create Vulkan image");
        }

        // Allocate command buffer
        if (!m_AssetsCommandPool) {
            m_AssetsCommandPool = renderer::getRen().getDevice().createCommandPool({ {}, renderer::getRen().getDeviceWrapper().getQueueManager().getGraphicsQueueFamilyIndex() });
        }
        vk::CommandBufferAllocateInfo allocInfo(m_AssetsCommandPool, vk::CommandBufferLevel::ePrimary, 1);
        vk::CommandBuffer commandBuffer = renderer::getRen().getDevice().allocateCommandBuffers(allocInfo)[0];

        vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        commandBuffer.begin(beginInfo);

        // Transition image layout and copy buffer data
        imageOut->transitionImageLayout(commandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal);

        commandBuffer.end();

        vk::SubmitInfo submitInfo({}, {}, commandBuffer);
        vk::FenceCreateInfo fenceInfo;
        vk::Fence fence = renderer::getRen().getDevice().createFence(fenceInfo);
        renderer::getRen().getDeviceWrapper().getQueueManager().submitGraphics(submitInfo, fence);
        RFCT_VULKAN_CHECK(renderer::getRen().getDevice().waitForFences(fence, VK_TRUE, UINT64_MAX));

        renderer::getRen().getDevice().freeCommandBuffers(m_AssetsCommandPool, commandBuffer);
        renderer::getRen().getDevice().destroyFence(fence);

        // Create Image View
        vk::ImageViewCreateInfo viewInfo({}, imageOut->m_image, vk::ImageViewType::e2D, vk::Format::eR8G8B8A8Unorm, {},
            { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
        imageOut->m_imageView = renderer::getRen().getDevice().createImageView(viewInfo);
    }

    vk::CommandPool& AssetsManager::getCommandPool()
    {
        if (!m_AssetsCommandPool) {
            m_AssetsCommandPool = renderer::getRen().getDevice().createCommandPool({ {}, renderer::getRen().getDeviceWrapper().getQueueManager().getGraphicsQueueFamilyIndex() });
        }
        return m_AssetsCommandPool;
    }

}