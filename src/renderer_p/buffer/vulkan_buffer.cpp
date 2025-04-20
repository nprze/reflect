#include "vulkan_buffer.h"
#include "renderer_p/renderer.h"

rfct::VulkanBuffer::VulkanBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags requiredFlags, VmaAllocationCreateFlags allocFlags)
{
    VmaAllocator allocator = renderer::getRen().getAllocator();
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = static_cast<VkBufferUsageFlags>(usage);
    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage = memoryUsage;
    allocCreateInfo.requiredFlags = requiredFlags;
    allocCreateInfo.flags = allocFlags;



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
    VmaAllocator allocator = renderer::getRen().getAllocator();

    if (allocation) {
        vmaDestroyBuffer(allocator, static_cast<VkBuffer>(buffer), allocation);
    }
}

void* rfct::VulkanBuffer::Map()
{
    VmaAllocator allocator = renderer::getRen().getAllocator();
    
    void* m_mappedData = nullptr;
    VkResult res = vmaMapMemory(allocator, allocation, &m_mappedData);
    if (res != VK_SUCCESS) {
        RFCT_CRITICAL("Failed to map Vulkan buffer memory.");
    }
    return m_mappedData;
}

void rfct::VulkanBuffer::Unmap()
{
    VmaAllocator allocator = renderer::getRen().getAllocator();

    vmaUnmapMemory(allocator, allocation);
}

void rfct::VulkanBuffer::CopyData(const void* data, size_t size)
{
    void* m_mappedData = Map();

    std::memcpy(m_mappedData, data, size);

    Unmap();
}
