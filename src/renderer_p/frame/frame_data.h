#pragma once
#include <vulkan\vulkan.hpp>
#include <tbb/concurrent_queue.h>
#include <vma\vk_mem_alloc.h>
namespace rfct {

    class frameData {
    public:
        frameData(vk::Device device, VmaAllocator& allocator);
        ~frameData() {};


        vk::CommandBuffer getCommandBuffer() const { return m_commandBuffer.get(); }
        vk::Fence getFence() const { return m_inRenderFence.get(); }
        const vk::Semaphore& getImageAvailableSemaphore() const { return m_imageAvailableSemaphore.get(); }
        const vk::Semaphore& getRenderFinishedSemaphore() const { return m_renderFinishedSemaphore.get(); }


        vk::SubmitInfo submitInfo();
    private:
        vk::Device m_device;
        VmaAllocator& m_allocator;

        vk::UniqueCommandPool m_commandPool;
        vk::UniqueCommandBuffer m_commandBuffer;
        vk::UniqueFence m_inRenderFence;
        vk::UniqueSemaphore m_imageAvailableSemaphore;
        vk::UniqueSemaphore m_renderFinishedSemaphore;

        vk::UniqueDescriptorPool m_descriptorPool;
        friend class renderer;
    };
} // namespace rfct