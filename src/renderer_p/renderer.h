#pragma once
#include "platform_p\windows\windows_window.h"
#include "device\vulkan_instance.h"
#include "device\vulkan_device.h"
namespace rfct {
	class renderer {
	public:
		static renderer ren;
	public:
		inline vk::Device getDevice() { return m_device.getDevice(); }
		inline vk::Instance getInstance() { return m_instance.getInstance(); }
		inline windowsWindow& getWindow() { return m_window; }
		inline vulkanInstance& getInstanceWrapper() { return m_instance; }
		renderer();
		~renderer() {};
		void run();
	private:
		windowsWindow m_window;
		vulkanInstance m_instance;
		vulkanDevice m_device;
	};
}