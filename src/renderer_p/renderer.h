#pragma once
#include "platform_p\desktop\glfw_window.h"
#include "device\vulkan_instance.h"
#include "device\vulkan_device.h"
#include "renderer_p\rasterizer_pipeline\vulkan_rasterizer_pipeline.h"
#include "renderer_p\frame\frame_resource_manager.h"
#include "renderer_p\ray_tracing\ray_tracer.h"
#include "renderer_p\rasterizer_pipeline\vertex.h"
#include "renderer_p\buffer\vulkan_vertex_buffer.h"
namespace rfct {
	struct allocator
	{
		VmaAllocator m_allocator;
		allocator();
		~allocator();
	};
	class renderer {
	public:
		static renderer ren;
	public:
		inline vk::Device getDevice() { return m_device.getDevice(); }
		inline vulkanDevice& getDeviceWrapper() { return m_device; }
		inline vk::Instance getInstance() { return m_instance.getInstance(); }
		inline windowAbstact& getWindow() { return *m_window; }
		inline vulkanInstance& getInstanceWrapper() { return m_instance; }
		inline vulkanRasterizerPipeline& getRasterizerPipeline() { return m_rasterizerPipeline; }
		inline VmaAllocator& getAllocator() { return m_allocator.m_allocator; }
		inline vulkanVertexBuffer& getVertexBuffer() { return m_vertexBuffer; }
		renderer();
		~renderer();
		void showWindow();
		void render();
		void setObjectName(void* objectHandle, const std::string& name, vk::ObjectType objectType);
	private:
	private:
		unique<windowAbstact> m_window;
		vulkanInstance m_instance;
		vulkanDevice m_device;
		vulkanRasterizerPipeline m_rasterizerPipeline;
		allocator m_allocator;
		framesInFlight m_framesInFlight;
		rayTracer m_rayTracer;
		vulkanVertexBuffer m_vertexBuffer;
	};
}