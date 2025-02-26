#pragma once
#include "vma\vk_mem_alloc.h"	
#include "vulkan\vulkan.hpp"
namespace rfct {
	struct VulkanBuffer {
        VulkanBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage);
        ~VulkanBuffer();

        void* Map();
        void Unmap();
        void CopyData(const void* data, size_t size);

        vk::Buffer buffer;
		VmaAllocation allocation;
	};;
}