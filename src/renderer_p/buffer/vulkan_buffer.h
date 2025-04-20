#pragma once
#include <vma/vk_mem_alloc.h>

namespace rfct {
	struct VulkanBuffer {
        VulkanBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VmaAllocationCreateFlags allocFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT);
        VulkanBuffer() = default;
        ~VulkanBuffer();


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
	};;
}