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
#include "assets/dialogue_serialize_data.h"

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

    void AssetsManager::loadScene(const std::string& path, sceneSerializedData* out)
    {
        // IO only here
        std::ifstream file(m_Path + "/" + path);
        if (!file.is_open()) {
            RFCT_CRITICAL("Failed to open scene file: {}", m_Path + "/" + path);
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::string_view sv(line);

            
            if (sv.starts_with("Rect:")) {
                rectangle r{};
                std::getline(file, line); // color
                r.color = line.substr(line.find(':') + 2);

                std::getline(file, line); // min
                {
                    int x, y;
                    std::from_chars(line.data() + 8, line.data() + line.find(','), x);
                    std::from_chars(line.data() + line.find(',') + 2, line.data() + line.size() - 1, y);
                    r.min = { x, y };
                }

                std::getline(file, line); // max
                {
                    int x, y;
                    std::from_chars(line.data() + 8, line.data() + line.find(','), x);
                    std::from_chars(line.data() + line.find(',') + 2, line.data() + line.size() - 1, y);
                    r.max = { x, y };
                }

                std::getline(file, line); // file
                r.file = line.substr(line.find(':') + 2);
                if (!r.file.empty() && r.file.back() == '\r') {
                    r.file.pop_back();
                }

                out->rectangles.push_back(std::move(r));
            }
            else if (sv.starts_with("Vine:")) {
                vineInfo vn = {};
                bool vineFullyOk = false;
                float xin, yin;
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
                out->vines.push_back(vn);
            }
            else if (sv.starts_with("NPC:")) {
                NPCInfo npc{};

                std::getline(file, line); // min
                {
                    float x, y;
                    std::from_chars(line.data() + 10, line.data() + line.find(','), x);
                    std::from_chars(line.data() + line.find(',') + 2, line.data() + line.size() - 1, y);
                    npc.min = { x, y };
                }

                std::getline(file, line); // max
                {
                    float x, y;
                    std::from_chars(line.data() + 8, line.data() + line.find(','), x);
                    std::from_chars(line.data() + line.find(',') + 2, line.data() + line.size() - 1, y);
                    npc.max = { x, y };
                }

                std::getline(file, line); // interaction radius
                float count;
                RFCT_ASSERT(sscanf(line.c_str(), "  interactionRadius: %f", &count) == 1);
                npc.ineratcionRadius = count;


                std::getline(file, line); // file
                npc.dialogueFile = line.substr(line.find(':') + 2);
                if (!npc.dialogueFile.empty() && npc.dialogueFile.back() == '\r') {
                    npc.dialogueFile.pop_back();
                }

                out->npcs.push_back(std::move(npc));
            }
            else if (sv.starts_with("SceneWidth:")) {
                std::from_chars(sv.data() + 12, sv.data() + sv.size(), out->width);
            }
            else if (sv.starts_with("SceneHeight:")) {
                std::from_chars(sv.data() + 13, sv.data() + sv.size(), out->height);
            }
            else if (sv.starts_with("RectCount:")) {
                int count;
                std::from_chars(sv.data() + 11, sv.data() + sv.size(), count);
                out->rectangles.reserve(count);
            }
            else if (sv.starts_with("VineCount:")) {
                int count;
                std::from_chars(sv.data() + 11, sv.data() + sv.size(), count);
                out->vines.reserve(count);
            }
            else if (sv.starts_with("NPCCount:")) {
                int count;
                std::from_chars(sv.data() + 10, sv.data() + sv.size(), count);
                out->npcs.reserve(count);
            }
            else if (sv.starts_with("SpawnPoint:")) {
                float xin, yin;
                RFCT_ASSERT(sscanf(line.c_str(), "SpawnPoint: (%f, %f)", &xin, &yin) == 2);
                out->spawnPoint.x = xin;
                out->spawnPoint.y = yin;
            }
        }
    }

    void AssetsManager::loadDialogue(const std::string& path, dialogueSerializeData* dialogueSerializedDataOut)
    {
        std::string finalPath = m_Path + "/" + path;
        std::ifstream file(finalPath);
        if (!file.is_open()) {
            RFCT_CRITICAL("Could not open file:  {}", finalPath);
        }

        enum class ParseState { None, Participants, DialogueText };
        ParseState state = ParseState::None;

        std::string line;
        dialogueParticipantSerializeData currentParticipant;

        auto trim = [](std::string& s) {
            auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
            s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
            };

        while (std::getline(file, line)) {
            std::string_view sv(line);
            trim(line);

            if (line.empty()) continue;

            // Version check
            if (sv.find("Version") != std::string::npos) {
                if (line != "Version 1.0") {
                    RFCT_CRITICAL("Unsupported dialogue version: {}", line);
                }
                continue;
            }

            if (line == "Participants:") {
                state = ParseState::Participants;
                continue;
            }
            if (line == "DialogueText:") {
                if (!currentParticipant.name.empty()) {
                    dialogueSerializedDataOut->participants.push_back(currentParticipant);
                    currentParticipant = {};
                }
                state = ParseState::DialogueText;
                continue;
            }

            if (state == ParseState::Participants) {
                if (line.back() == ':') {
                    // New participant
                    if (!currentParticipant.name.empty()) {
                        dialogueSerializedDataOut->participants.push_back(currentParticipant);
                        currentParticipant = {};
                    }
                    currentParticipant.name = line.substr(0, line.size() - 1);
                    trim(currentParticipant.name);
                }
                else {
                    // Sprite filename
                    currentParticipant.spritesFilenames.push_back(line);
                }
            }
            else if (state == ParseState::DialogueText) {
                // Format: {participant data}dialogue text
                auto openBrace = line.find('{');
                auto closeBrace = line.find('}');

                if (openBrace == std::string::npos || closeBrace == std::string::npos || closeBrace < openBrace) {
                    RFCT_CRITICAL("Malformed dialogue text line: {}", line);
                }

                dialogueNodeSerializeData node;
                node.participantDataInBrackets = line.substr(openBrace + 1, closeBrace - openBrace - 1);
                trim(node.participantDataInBrackets);

                node.dialogueText = line.substr(closeBrace + 1);
                trim(node.dialogueText);

                dialogueSerializedDataOut->text.push_back(std::move(node));
            }
        }

        if (!currentParticipant.name.empty()) {
            dialogueSerializedDataOut->participants.push_back(currentParticipant);
        }

    }

    void AssetsManager::loadDialogueSpriteSheet(const std::string& path, dialogueSpritesheetSerializeData* dialogueSpritesheetSerializedDataOut)
    {
        std::string finalPath = m_Path + "/" + path;
        std::ifstream file(finalPath);
        if (!file.is_open()) {
            RFCT_CRITICAL("Could not open file:  {}", finalPath);
        }


        std::string line;
        spritesheetCycle currentCycle;
        std::string currentCycleName;

        // First read row/column counts
        while (std::getline(file, line)) {
            if (line.empty())
                continue;

            std::istringstream iss(line);
            std::string key;
            iss >> key;

            if (key == "ColumnCount:") {
                iss >> dialogueSpritesheetSerializedDataOut->columnCount;
            }
            else if (key == "RowCount:") {
                iss >> dialogueSpritesheetSerializedDataOut->rowCount;
            }
            else if (key == "CycleName:") {
                // Save the previous cycle if there was one
                if (!currentCycleName.empty()) {
                    dialogueSpritesheetSerializedDataOut->cycles[currentCycleName] = currentCycle;
                    currentCycle = spritesheetCycle(); // reset
                }
                iss >> currentCycleName;
            }
            else if (key == "Images:") {
                currentCycle.indices.clear();
                std::string token;
                while (iss >> token) {
                    if (token.front() == '(' && token.back() == ')') {
                        token = token.substr(1, token.size() - 2); // strip ()
                        size_t commaPos = token.find(',');
                        if (commaPos != std::string::npos) {
                            int row = std::stoi(token.substr(0, commaPos));
                            int col = std::stoi(token.substr(commaPos + 1));
                            currentCycle.indices.emplace_back(row, col);
                        }
                    }
                }
            }
            else if (key == "Repeat:") {
                iss >> currentCycle.repeat;
            }
            else if (key == "CycleTime:") {
                iss >> currentCycle.cycleTime;
            }
            else if (key == "Fallback:") {
                iss >> currentCycle.fallBack;
            }
        }

        // Store the last cycle if there was one
        if (!currentCycleName.empty()) {
            dialogueSpritesheetSerializedDataOut->cycles[currentCycleName] = currentCycle;
        }

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