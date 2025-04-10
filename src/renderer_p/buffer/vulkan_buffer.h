#pragma once
#include "vma\vk_mem_alloc.h"	
#include "vulkan\vulkan.hpp"
namespace rfct {
	struct VulkanBuffer {
        VulkanBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage);
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