#pragma once
#include "device\vulkan_instance.h"
#include "device\vulkan_device.h"
#include "renderer_p\rasterizer_pipeline\vulkan_rasterizer_pipeline.h"
#include "renderer_p\frame\frame_resource_manager.h"
#include "renderer_p\rasterizer_pipeline\vertex.h"
#include "renderer_p\buffer\vulkan_vertex_buffer.h"
#include "window.h"
#include "platform_window.h"
#include "renderer_p\debug\debug_draw.h"
#include "assets\assets_manager.h"
#include "renderer_p\UI\ui_pipeline.h"
namespace rfct {
	struct SurfaceWrapper {
		vk::SurfaceKHR surface;
        SurfaceWrapper(vk::SurfaceKHR surfaceArg);
        void newSurface(vk::SurfaceKHR surfaceArg);
		~SurfaceWrapper();
	};
	struct allocator
	{
		VmaAllocator m_allocator;
		allocator();
		~allocator();
	};
	class renderer {
    public:
        static renderer& getRen() {return *ren;};
    private:
		static renderer* ren;
	public:
		inline vk::Device getDevice() { return m_device.getDevice(); }
		inline vulkanDevice& getDeviceWrapper() { return m_device; }
		inline vk::Instance getInstance() { return m_instance.getInstance(); }
		inline RFCT_PLATFORM_WINDOW& getWindow() { return m_window; }
		inline vulkanInstance& getInstanceWrapper() { return m_instance; }
		inline vulkanRasterizerPipeline& getRasterizerPipeline() { return m_rasterizerPipeline; }
		inline VmaAllocator& getAllocator() { return m_allocator.m_allocator; }
        inline AssetsManager* getAssetsManager() { return m_AssetsManager; }
		inline vk::SurfaceKHR& getSurface() { return m_surface.surface; }
		inline float getAspectRatio() { return m_window.getAspectRatio(); }
		inline vk::CommandPool getAssetsCommandPool() { return m_AssetsCommandPool.get(); }
		inline UIPipeline& getUIPipeline() { return m_UIPipeline; };
        void updateWindow(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR);
        renderer(RFCT_RENDERER_ARGUMENTS);
		~renderer();
		void showWindow();
		void render(frameContext& frameContext);
		void setObjectName(void* objectHandle, const std::string& name, vk::ObjectType objectType);
	private:
	private:
		AssetsManager* m_AssetsManager;
        RFCT_PLATFORM_WINDOW m_window;
		vulkanInstance m_instance;
		SurfaceWrapper m_surface;
		vulkanDevice m_device;
		vk::UniqueCommandPool m_AssetsCommandPool;
		vulkanRasterizerPipeline m_rasterizerPipeline;
		allocator m_allocator;
		framesInFlight m_framesInFlight;
		debugDraw m_debugDraw;
		UIPipeline m_UIPipeline;
    private:
        friend class vulkanSwapChain;
        friend class reflectApplication;
		friend AssetsManager* setStaticRenderer(renderer* rendererArg, AssetsManager* assetsManager);
	};
}