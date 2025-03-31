#pragma once
#include <vulkan\vulkan.hpp>
#include <vma\vk_mem_alloc.h>
#include "renderer_p\descriptors\camera_ubo.h"
#include "renderer_p\descriptors\per_frame_descriptors.h"
#include "world_p\camera\camera.h"
namespace rfct {

    class frameData {
    public:
        frameData(vk::Device device, VmaAllocator& allocator);
        ~frameData() {};

		void prepareFrame();

        vk::CommandBuffer getSceneCommandBuffer() const { return m_sceneCommandBuffer.get(); }
        vk::CommandBuffer getDebugDrawCommandBuffer() const { return m_debugDrawCommandBuffer.get(); }
        vk::CommandBuffer getuiCommandBuffer() const { return m_uiCommandBuffer.get(); }
        vk::Fence getSceneInRenderFence() const { return m_sceneInRenderFence.get(); }
        vk::Fence getdebugDrawInRenderFence() const { return m_debugDrawTrianglesInRenderFence.get(); }
        vk::Fence getuiInRenderFence() const { return m_uiInRenderFence.get(); }
        const vk::Semaphore& getImageAvailableSemaphore() const { return m_imageAvailableSemaphore.get(); }
        const vk::Semaphore& getRenderFinishedSemaphore() const { return m_renderFinishedSemaphore.get(); }
        void waitForAllFences();
        void resetAllFences();
		vk::DescriptorSet getCameraUboDescSet() const { return m_descriptors.m_cameraUboDescSet.get(); }
		vk::DescriptorSet getUICameraUboDescSet() const { return m_UIcameradescriptors.m_cameraUboDescSet.get(); }

        vk::SubmitInfo sceneSubmitInfo();
        vk::SubmitInfo debugDrawSubmitInfo();
        vk::SubmitInfo uiSubmitInfo();
    private:
        vk::Device m_device;
        VmaAllocator& m_allocator;

        vk::UniqueCommandPool m_commandPool;
        vk::UniqueCommandBuffer m_sceneCommandBuffer;
        vk::UniqueFence m_sceneInRenderFence;
        vk::UniqueSemaphore m_imageAvailableSemaphore;
        vk::UniqueSemaphore m_renderFinishedSemaphore;

        vk::UniqueCommandBuffer m_debugDrawCommandBuffer;
        vk::UniqueFence m_debugDrawTrianglesInRenderFence;

        vk::UniqueCommandBuffer m_uiCommandBuffer;
        vk::UniqueFence m_uiInRenderFence;


		cameraUbo m_cameraUbo;
		cameraUbo m_UIcameraUbo;

		descriptors m_descriptors;
		descriptors m_UIcameradescriptors;

        friend class renderer;
        friend class debugDraw;
        friend class UIPipeline;
    };
} // namespace rfct