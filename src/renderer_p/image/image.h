#pragma once
#include <vulkan\vulkan.hpp>
#include <vma\vk_mem_alloc.h>

namespace rfct {
    class image {
    public:
        image(const std::string& path);
        ~image();
        void transitionImageLayout(vk::CommandBuffer commandBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
        void copyBufferToImage(vk::CommandBuffer commandBuffer, vk::Buffer buffer);

    public:
        uint32_t width;
        uint32_t height;
        vk::Image m_image;
        vk::ImageView m_imageView;
        VmaAllocation m_allocation;
    };
}