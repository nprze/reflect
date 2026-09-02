#pragma once
#include "context.h"
#include "renderer_p/descriptors/ubo.h"
#include "renderer_p/descriptors/camera_descriptors.h"
#include <vulkan/vulkan.hpp>

namespace rfct {
	class RfctVulkanMemAllocator;
    class frameData {
    public:
        frameData(RfctVulkanMemAllocator& allocatorWrapper, RfctQueue& queue, vk::Device device, vk::Fence lastFramePresentFinishedFence, vk::Fence thisFramePresentFinishedFence);
		void prepareFrame(const frameContext& ctx, uint32_t BufferIndex, float changeSceneEffectMultiplier);
        void WaitForFences(vk::Device device);
        void ResetFences(vk::Device device);
		vk::DescriptorSet& getCameraUboDescSet(uint32_t BufferIndex) { return m_descriptors.getCameraDescSet(BufferIndex); }
		vk::DescriptorSet& getUICameraUboDescSet() { return m_UIcameradescriptors.getCameraDescSet(0); }
        vk::SubmitInfo sceneSubmitInfo(const frameContext& ctx) const;
        vk::SubmitInfo bloomSubmitInfo(const frameContext& ctx) const;
        vk::SubmitInfo debugDrawSubmitInfo(const frameContext& ctx) const;
        vk::SubmitInfo uiSubmitInfo(const frameContext& ctx) const;
    private:
        vk::UniqueCommandPool m_sceneCommandPool;
        vk::UniqueCommandBuffer m_sceneCommandBuffer;
        vk::UniqueSemaphore m_sceneFinishedSemaphore;

        vk::UniqueSemaphore m_ImageAvaibleSemaphore;

        vk::CommandBuffer m_BloomCommandBuffer;
        vk::UniqueSemaphore m_bloomFinishedSemaphore;

        vk::UniqueCommandPool m_debugDrawCommandPool;
        vk::UniqueCommandBuffer m_debugDrawCommandBuffer;
        vk::UniqueSemaphore m_debugDrawFinishedSemaphore;

        vk::UniqueCommandPool m_uiCommandPool;
        vk::UniqueCommandBuffer m_uiCommandBuffer;
        vk::UniqueSemaphore m_renderFinishedSemaphore;

        vk::UniqueFence m_renderingFence;

		vk::Fence m_thisFrameRenderFinishedFence;
		vk::Fence m_lastFrameRenderFinishedFence;

		std::array<ubo, RFCT_FRAMES_IN_FLIGHT> m_cameraUbo;
		ubo m_UIcameraUbo;

		descriptors m_descriptors;
		descriptors m_UIcameradescriptors;
    private:
        friend class RfctRenderer;
        friend class debugDraw;
        friend class UIPipelines;
        friend class vulkanRasterizerPipeline;
        friend class bloomResurcesHolder;
    };
} // namespace rfct