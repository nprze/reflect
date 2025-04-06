#include "assets_manager.h"
#include "app.h"
#include "stb_image\stb_image.h"
namespace rfct {
    AssetsManager::AssetsManager(std::string path){
        if (path.length()!=0) {
            m_Path = path;
            return;
        }else{
            m_Path = std::string(RFCT_ASSETS_DIR);
        }
    }
    AssetsManager::~AssetsManager()
    {
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
        vk::CommandBufferAllocateInfo allocInfo(renderer::getRen().getAssetsCommandPool(), vk::CommandBufferLevel::ePrimary, 1);
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

        renderer::getRen().getDevice().freeCommandBuffers(renderer::getRen().getAssetsCommandPool(), commandBuffer);
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
                int id;
                glyph g;
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

}