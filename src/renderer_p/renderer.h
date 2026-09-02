#pragma once
#include "platform_window.h"
#include "components/renderer_components.h"
#include "renderer_p/frame/frame_resource_manager.h"
#include "renderer_p/frame/render_target_manager.h"
#include "renderer_p/debug/debug_draw.h"
#include "renderer_p/rasterizer_pipeline/vulkan_rasterizer_pipeline.h"
#include "renderer_p/UI/ui_pipeline.h"
#include "renderer_p/post_process/bloom.h"

namespace rfct {
	class RfctRenderer {
	public:
		vk::Device& GetDevice() { return m_device.GetDevice(); }
		RfctDevice& GetDeviceWrapper() { return m_device; }
		vk::Instance& GetInstance() { return m_instance.GetInstance(); }
		RFCT_PLATFORM_WINDOW& GetWindow() { return m_window; }
		RfctVulkanInstance& GetInstanceWrapper() { return m_instance; }
		renderImagesManager& GetRenderImagesManager() { return m_renderImages; }
		vulkanRasterizerPipeline& GetRasterizerPipeline() { return m_rasterizerPipeline; }
		bloomResurcesHolder& GetBloomRes() { return m_bloomRes; }
		VmaAllocator& GetAllocator() { return m_allocator.GetAllocator(); }
		vk::SurfaceKHR GetSurface() { return m_surface.GetSurface(); }
		float GetAspectRatio() { return m_window.GetAspectRatio(); }
		vk::Extent2D GetExtent() { return m_window.GetExtent(); }
		UIPipelines& GetUIPipeline() { return m_UIPipeline; };

        RfctRenderer(RFCT_RENDERER_ARGUMENTS);
		~RfctRenderer();
        void UpdateWindow(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR);
		void Render(frameContext& frameContext);
		void SetObjectName(void* objectHandle, const std::string& name, vk::ObjectType objectType);
	private:
        RFCT_PLATFORM_WINDOW m_window;
		RfctVulkanInstance m_instance;
		RfctSurfaceWrapper m_surface;
		RfctDevice m_device;
		RfctQueue m_queue;
		RfctVulkanMemAllocator m_allocator;
		RfctSwapChain m_swapChain;
		renderImagesManager m_renderImages;
		framesInFlight m_framesInFlight;
		vulkanRasterizerPipeline m_rasterizerPipeline;
		bloomResurcesHolder m_bloomRes;
		debugDraw m_debugDraw;
		UIPipelines m_UIPipeline;
	};
	RfctRenderer& GetRen();
}