#include "image_load.h"
#include "renderer_p/image/image.h"
#include "renderer_p/UI/font/font.h"
#include "serialize_structures/button_image_serialize_data.h"

#include "renderer_p/renderer.h"
#include "assets_utils.h"
#include "stb_image/stb_image.h"
#include <fstream>

void rfct::loadImage(const std::string& path, image* imageOut)
{
    std::string finalPath = getAssetsPath() + "/" + path;
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

    vk::CommandBufferAllocateInfo allocInfo(getAssetsCommandPool(), vk::CommandBufferLevel::ePrimary, 1);
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

    renderer::getRen().getDevice().freeCommandBuffers(getAssetsCommandPool(), commandBuffer);
    vmaDestroyBuffer(renderer::getRen().getAllocator(), static_cast<VkBuffer>(stagingBuffer), stagingBufferAllocation);
    renderer::getRen().getDevice().destroyFence(fence);

    // Create Image View
    vk::ImageViewCreateInfo viewInfo({}, imageOut->m_image, vk::ImageViewType::e2D, vk::Format::eR8G8B8A8Unorm, {},
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    imageOut->m_imageView = renderer::getRen().getDevice().createImageView(viewInfo);
}

void rfct::createDummyImage(image* imageOut)
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
    vk::CommandBufferAllocateInfo allocInfo(getAssetsCommandPool(), vk::CommandBufferLevel::ePrimary, 1);
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

    renderer::getRen().getDevice().freeCommandBuffers(getAssetsCommandPool(), commandBuffer);
    renderer::getRen().getDevice().destroyFence(fence);

    // Create Image View
    vk::ImageViewCreateInfo viewInfo({}, imageOut->m_image, vk::ImageViewType::e2D, vk::Format::eR8G8B8A8Unorm, {},
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
    imageOut->m_imageView = renderer::getRen().getDevice().createImageView(viewInfo);
}

void rfct::loadGlyphs(const std::string& path, font* fontOut)
{
    std::ifstream file;
    if (!openAssetFile(path, &file)) {
        RFCT_CRITICAL("Failed to open font data file: {}", path);
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

void rfct::loadButtonImage(const std::string& path, buttonImageSerializeData* buttonImageSerializedDataOut)
{
    std::ifstream file;
    if (!openAssetFile(path, &file)) {
        RFCT_CRITICAL("Failed to open button image descripting file: {}", path);
    }

    std::string line;
    int currentRow = -1;
    bool parsingReleased = false;
    bool parsingHold = false;

    std::unordered_map<std::string, buttonCoordInfo*> buttonMap = {
        {"joystickButton",   &buttonImageSerializedDataOut->joystick},
        {"hold",   &buttonImageSerializedDataOut->hold},
        {"jump",   &buttonImageSerializedDataOut->jump},
        {"menu",   &buttonImageSerializedDataOut->menu},
        {"dash",   &buttonImageSerializedDataOut->dash}
    };

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key == "image:") {
            iss >> buttonImageSerializedDataOut->imagePath;
        }
        else if (key == "joystickBG:") {
            iss >> buttonImageSerializedDataOut->joystickImagePath;
        }
        else if (key == "imageRows:") {
            iss >> buttonImageSerializedDataOut->imageRows;
        }
        else if (key == "imageColumns:") {
            iss >> buttonImageSerializedDataOut->imageColumns;
        }
        else if (key == "buttonSize:") {
            char c;
            float w, h;
            iss >> c >> w >> c >> h >> c;
            buttonImageSerializedDataOut->buttonSize = { w, h };
        }
        else if (key == "Row") {
            iss >> currentRow;
        }
        else if (key == "Released:") {
            parsingReleased = true;
            parsingHold = false;
        }
        else if (key == "Hold:") {
            parsingReleased = false;
            parsingHold = true;
        }
        else {
            auto it = buttonMap.find(key);
            if (it != buttonMap.end()) {
                int colIndex = 0;
                static int colCounter = 0;

                if (parsingReleased) {
                    it->second->released = { currentRow, colCounter };
                }
                else if (parsingHold) {
                    it->second->hold = { currentRow, colCounter };
                }

                colCounter++;

                if (file.peek() == '\n') colCounter = 0;
            }
        }
    }
}