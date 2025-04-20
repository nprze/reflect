#pragma once
#include "platform_window.h"
#include "device/vulkan_instance.h"
#include "device/vulkan_device.h"
#include "renderer_p/frame/frame_resource_manager.h"

#include "renderer_p/rasterizer_pipeline/vulkan_rasterizer_pipeline.h"
#include "renderer_p/debug/debug_draw.h"
#include "renderer_p/UI/ui_pipeline.h"

namespace rfct {
	struct SurfaceWrapper { // surface works a little bit sussy on android (this handles the calls java makes on surface holder change)
		vk::SurfaceKHR surface;
        SurfaceWrapper(vk::SurfaceKHR surfaceArg);
        void newSurface(vk::SurfaceKHR surfaceArg);
		~SurfaceWrapper();
	};
	struct allocator // vma is static on windows, dynamic on android (requires different setups).
	{
		VmaAllocator m_allocator;
		allocator();
		~allocator();
	};
	class renderer {
    private:
		static renderer* ren;
    public:
        static renderer& getRen() {return *ren;};
	public:
		inline vk::Device& getDevice() { return m_device.getDevice(); }
		inline vulkanDevice& getDeviceWrapper() { return m_device; }
		inline vk::Instance& getInstance() { return m_instance.getInstance(); }
		inline RFCT_PLATFORM_WINDOW& getWindow() { return m_window; }
		inline vulkanInstance& getInstanceWrapper() { return m_instance; }
		inline vulkanRasterizerPipeline& getRasterizerPipeline() { return m_rasterizerPipeline; }
		inline VmaAllocator& getAllocator() { return m_allocator.m_allocator; }
		inline vk::SurfaceKHR& getSurface() { return m_surface.surface; }
		inline float getAspectRatio() { return m_window.getAspectRatio(); }
		inline UIPipeline& getUIPipeline() { return m_UIPipeline; };

        renderer(RFCT_RENDERER_ARGUMENTS);
		~renderer();
        void updateWindow(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR);

		void render(frameContext& frameContext);
		void setObjectName(void* objectHandle, const std::string& name, vk::ObjectType objectType);
	private:
		bool m_uselessBool; // do not remove
        RFCT_PLATFORM_WINDOW m_window;
		vulkanInstance m_instance;
		SurfaceWrapper m_surface; // here for simpler access when surface holder changes (android)
		vulkanDevice m_device;
		vulkanRasterizerPipeline m_rasterizerPipeline;
		allocator m_allocator;
		framesInFlight m_framesInFlight;
		debugDraw m_debugDraw;
		UIPipeline m_UIPipeline;
    private:
        friend class vulkanSwapChain;
        friend class reflectApplication;
		friend bool setStaticRenderer(renderer* rendererArg);
	};
}