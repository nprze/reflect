#pragma once
#include <vulkan\vulkan.hpp>
#include <vma\vk_mem_alloc.h>
namespace rfct {

    class frameData {
    public:
        frameData(vk::Device device, VmaAllocator& allocator);
        ~frameData() {};


        vk::CommandBuffer getSceneCommandBuffer() const { return m_sceneCommandBuffer.get(); }
        vk::CommandBuffer getDebugTrianglesCommandBuffer() const { return m_debugDrawTrianglesCommandBuffer.get(); }
        vk::Fence getFence() const { return m_sceneInRenderFence.get(); }
        const vk::Semaphore& getImageAvailableSemaphore() const { return m_imageAvailableSemaphore.get(); }
        const vk::Semaphore& getRenderFinishedSemaphore() const { return m_renderFinishedSemaphore.get(); }
        void waitForAllFences();
        void resetAllFences();

        vk::SubmitInfo submitInfo();
    private:
        vk::Device m_device;
        VmaAllocator& m_allocator;

        vk::UniqueCommandPool m_commandPool;
        vk::UniqueCommandBuffer m_sceneCommandBuffer;
        vk::UniqueFence m_sceneInRenderFence;
        vk::UniqueSemaphore m_imageAvailableSemaphore;
        vk::UniqueSemaphore m_renderFinishedSemaphore;

        vk::UniqueDescriptorPool m_descriptorPool;

        vk::UniqueCommandBuffer m_debugDrawTrianglesCommandBuffer;
        vk::UniqueFence m_debugDrawTrianglesInRenderFence;

        friend class renderer;
    };
} // namespace rfct