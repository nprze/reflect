#pragma once
#include <vulkan\vulkan.hpp>
#include <vma\vk_mem_alloc.h>
#include "renderer_p\descriptors\camera_ubo.h"
#include "renderer_p\descriptors\per_frame_descriptors.h"
#include "world_p\camera\camera.h"
namespace rfct {

    class frameData {
    public:
        frameData(vk::Device device, VmaAllocator& allocator, vk::Fence lastFramePresentFinishedFence, vk::Fence thisFramePresentFinishedFence);
        ~frameData() {};

		void prepareFrame();

        void waitForAllFences();
        void resetAllFences();
		vk::DescriptorSet getCameraUboDescSet() const { return m_descriptors.m_cameraUboDescSet.get(); }
		vk::DescriptorSet getUICameraUboDescSet() const { return m_UIcameradescriptors.m_cameraUboDescSet.get(); }

        vk::SubmitInfo sceneSubmitInfo() const;
        vk::SubmitInfo debugDrawSubmitInfo() const;
        vk::SubmitInfo uiSubmitInfo() const;


        vk::Device m_device;
        VmaAllocator& m_allocator;

        vk::UniqueCommandPool m_sceneCommandPool;
        vk::UniqueCommandBuffer m_sceneCommandBuffer;
        vk::UniqueSemaphore m_sceneFinishedSemaphore;

        vk::UniqueSemaphore m_ImageAvaibleSemaphore;

        vk::UniqueCommandPool m_debugDrawCommandPool;
        vk::UniqueCommandBuffer m_debugDrawCommandBuffer;
        //vk::UniqueSemaphore m_debugDrawTrigsFinishedSemaphore;
        vk::UniqueSemaphore m_debugDrawFinishedSemaphore;

        vk::UniqueCommandPool m_uiCommandPool;
        vk::UniqueCommandBuffer m_uiCommandBuffer;
        vk::UniqueSemaphore m_renderFinishedSemaphore;

        vk::UniqueFence m_renderingFence;

		vk::Fence m_thisFrameRenderFinishedFence;
		vk::Fence m_lastFrameRenderFinishedFence;

		cameraUbo m_cameraUbo;
		cameraUbo m_UIcameraUbo;

		descriptors m_descriptors;
		descriptors m_UIcameradescriptors;

    private:
        friend class renderer;
        friend class debugDraw;
        friend class UIPipeline;
    };
} // namespace rfct