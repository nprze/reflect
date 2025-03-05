#pragma once
#include "device\vulkan_instance.h"
#include "device\vulkan_device.h"
#include "renderer_p\rasterizer_pipeline\vulkan_rasterizer_pipeline.h"
#include "renderer_p\frame\frame_resource_manager.h"
#include "renderer_p\ray_tracing\ray_tracer.h"
#include "renderer_p\rasterizer_pipeline\vertex.h"
#include "renderer_p\buffer\vulkan_vertex_buffer.h"
#include "window.h"
#include "platform_window.h"
namespace rfct {
    struct uselessClass{
        bool state;
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
		inline vulkanVertexBuffer& getVertexBuffer() { return m_vertexBuffer; }
        renderer(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR);
		~renderer();
		void showWindow();
		void render();
		void setObjectName(void* objectHandle, const std::string& name, vk::ObjectType objectType);
	private:
	private:
        uselessClass uc;
        RFCT_PLATFORM_WINDOW m_window;
		vulkanInstance m_instance;
		vulkanDevice m_device;
		vulkanRasterizerPipeline m_rasterizerPipeline;
		allocator m_allocator;
		framesInFlight m_framesInFlight;
		rayTracer m_rayTracer;
		vulkanVertexBuffer m_vertexBuffer;
    private:
        friend class reflectApplication;
        friend uselessClass createUselessClass(renderer* rendererArg);
	};
}