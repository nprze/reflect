#pragma once
#include "platform_p\desktop\glfw_window.h"
#include "device\vulkan_instance.h"
#include "device\vulkan_device.h"
#include "renderer_p\rasterizer_pipeline\vulkan_rasterizer_pipeline.h"
#include "renderer_p\frame\frame_resource_manager.h"
#include "renderer_p\ray_tracing\ray_tracer.h"
namespace rfct {
	class renderer {
	public:
		static renderer ren;
	public:
		inline vk::Device getDevice() { return m_device.getDevice(); }
		inline vulkanDevice& getDeviceWrapper() { return m_device; }
		inline vk::Instance getInstance() { return m_instance.getInstance(); }
		inline GlfwWindow& getWindow() { return m_window; }
		inline vulkanInstance& getInstanceWrapper() { return m_instance; }
		inline vulkanRasterizerPipeline& getRasterizerPipeline() { return m_rasterizerPipeline; }
		renderer();
		~renderer() { m_device.getDevice().waitIdle(); };
		void showWindow();
		void render();
		void setObjectName(void* objectHandle, const std::string& name, vk::ObjectType objectType);
	private:
		GlfwWindow m_window;
		vulkanInstance m_instance;
		vulkanDevice m_device;
		vulkanRasterizerPipeline m_rasterizerPipeline;
		framesInFlight m_framesInFlight;
		rayTracer m_rayTracer;
	};
}