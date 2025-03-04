#include "vulkan_buffer.h"
#include "renderer_p\renderer.h"

rfct::VulkanBuffer::VulkanBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
    VmaAllocator allocator = renderer::ren->getAllocator();
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = static_cast<VkBufferUsageFlags>(usage);
    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage = memoryUsage;

    VkBuffer vkBuffer;
    VkResult res = vmaCreateBuffer(allocator, &bufferCreateInfo,
        &allocCreateInfo, &vkBuffer, &allocation, nullptr);
    if (res != VK_SUCCESS) {
        RFCT_CRITICAL("Buffer creation failed with code: {0}", (uint32_t)res);
    }

    buffer = vk::Buffer(vkBuffer);
}

rfct::VulkanBuffer::~VulkanBuffer()
{
    VmaAllocator allocator = renderer::ren->getAllocator();

    if (allocation) {
        vmaDestroyBuffer(allocator, static_cast<VkBuffer>(buffer), allocation);
    }
}

void* rfct::VulkanBuffer::Map()
{
    VmaAllocator allocator = renderer::ren->getAllocator();

    void* mappedData = nullptr;
    VkResult res = vmaMapMemory(allocator, allocation, &mappedData);
    if (res != VK_SUCCESS) {
        RFCT_CRITICAL("Failed to map Vulkan buffer memory.");
    }
    return mappedData;
}

void rfct::VulkanBuffer::Unmap()
{
    VmaAllocator allocator = renderer::ren->getAllocator();

    vmaUnmapMemory(allocator, allocation);
}

void rfct::VulkanBuffer::CopyData(const void* data, size_t size)
{
    void* mappedData = Map();

    std::memcpy(mappedData, data, size);

    Unmap();
}
