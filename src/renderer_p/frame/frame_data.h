#pragma once
#include "renderer_p/descriptors/camera_ubo.h"
#include "renderer_p/descriptors/camera_descriptors.h"
#include "context.h"

namespace rfct {

    class frameData {
    public:
        frameData(vk::Device device, VmaAllocator& allocator, vk::Fence lastFramePresentFinishedFence, vk::Fence thisFramePresentFinishedFence);
        ~frameData() {};

		void prepareFrame(uint32_t BufferIndex);

        void waitForFences();
        void resetFences();
		vk::DescriptorSet& getCameraUboDescSet(uint32_t BufferIndex) { return m_descriptors.getCameraDescSet(BufferIndex); }
		vk::DescriptorSet& getUICameraUboDescSet() { return m_UIcameradescriptors.getCameraDescSet(0); }

        vk::SubmitInfo sceneSubmitInfo(const frameContext& ctx) const;
        vk::SubmitInfo debugDrawSubmitInfo(const frameContext& ctx) const;
        vk::SubmitInfo uiSubmitInfo(const frameContext& ctx) const;
    private:
        vk::UniqueCommandPool m_sceneCommandPool;
        vk::UniqueCommandBuffer m_sceneCommandBuffer;
        vk::UniqueSemaphore m_sceneFinishedSemaphore;

        vk::UniqueSemaphore m_ImageAvaibleSemaphore;

        vk::UniqueCommandPool m_debugDrawCommandPool;
        vk::UniqueCommandBuffer m_debugDrawCommandBuffer;
        vk::UniqueSemaphore m_debugDrawFinishedSemaphore;

        vk::UniqueCommandPool m_uiCommandPool;
        vk::UniqueCommandBuffer m_uiCommandBuffer;
        vk::UniqueSemaphore m_renderFinishedSemaphore;

        vk::UniqueFence m_renderingFence;

		vk::Fence m_thisFrameRenderFinishedFence;
		vk::Fence m_lastFrameRenderFinishedFence;

		std::array<cameraUbo, RFCT_FRAMES_IN_FLIGHT> m_cameraUbo;
		cameraUbo m_UIcameraUbo; // one bcs when windows get resized, device waits idle anyway

		descriptors m_descriptors;
		descriptors m_UIcameradescriptors;

    private:
        friend class renderer;
        friend class debugDraw;
        friend class UIPipeline;
        friend class vulkanRasterizerPipeline;
    };
} // namespace rfct