#pragma once
#define VMA_DEBUG_DETECT_MAPPING_MISMATCH
#include <vma/vk_mem_alloc.h>

namespace rfct {
	struct VulkanBuffer {
        VulkanBuffer(const char* name, vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VmaAllocationCreateFlags allocFlags = 0);
        VulkanBuffer() = default;
        ~VulkanBuffer();
        VulkanBuffer(VulkanBuffer&& bffr) noexcept;
        VulkanBuffer& operator=(VulkanBuffer&& bffr) noexcept;

        void cleanupBuffer();
        void* Map();
        void Unmap();
        void CopyData(const void* data, size_t size);

        vk::Buffer buffer;
		VmaAllocation allocation;
	};
    struct vulkanBufferLocation {
        VulkanBuffer* buffer;
        uint32_t offsetInBytes;
    };
}