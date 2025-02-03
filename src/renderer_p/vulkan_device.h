#pragma once

namespace rfct {
	class VulkanDevice {
	private:
		static VulkanDevice device;
	public:
		static VulkanDevice& get() { return device; };
		static vk::Device& getDevice() { return *device.m_Device; };
	private:
		VulkanDevice();
	private:
		vk::UniqueDevice m_Device;
		vk::UniqueInstance m_Instance;
		vk::PhysicalDevice m_PhysicalDevice;
	};
}
