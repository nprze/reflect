#pragma once
#define VMA_DEBUG_DETECT_MAPPING_MISMATCH
#include <vma/vk_mem_alloc.h>

namespace rfct {
	struct VulkanBuffer {
        VulkanBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VmaAllocationCreateFlags allocFlags = 0);
        VulkanBuffer() = default;
        ~VulkanBuffer();
        void cleanup();

        // move constructor
        VulkanBuffer(VulkanBuffer&& bffr) noexcept
            : buffer(bffr.buffer), allocation(bffr.allocation)
        {
            bffr.buffer = nullptr;
            bffr.allocation = nullptr;
        }
        VulkanBuffer& operator=(VulkanBuffer&& bffr) noexcept {
            buffer = bffr.buffer;
			allocation = bffr.allocation;
			bffr.buffer = nullptr;
			bffr.allocation = nullptr;
            return *this;
        }

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