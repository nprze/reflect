#pragma once
#include "context.h"
#include <vulkan/vulkan.hpp>

namespace rfct {
	class RfctVulkanMemAllocator;
	class RfctQueue;
    class frameSyncDataTemp {
    public:
        frameSyncDataTemp(RfctVulkanMemAllocator& allocatorWrapper, RfctQueue& queue, vk::Device device, vk::Fence lastFramePresentFinishedFence, vk::Fence thisFramePresentFinishedFence);
        void WaitForFences(vk::Device device);
        void ResetFences(vk::Device device);
        vk::SubmitInfo sceneSubmitInfo(const frameContext& ctx) const;
        vk::SubmitInfo bloomSubmitInfo(const frameContext& ctx) const;
        vk::SubmitInfo debugDrawSubmitInfo(const frameContext& ctx) const;
        vk::SubmitInfo uiSubmitInfo(const frameContext& ctx) const;
    public:
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

    private:
        friend class RfctRenderer;
        friend class debugDraw;
        friend class UIPipelines;
        friend class vulkanRasterizerPipeline;
        friend class bloomResurcesHolder;
    };
} // namespace rfct