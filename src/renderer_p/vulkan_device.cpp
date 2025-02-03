#include "vulkan_device.h"

namespace rfct {
    VulkanDevice VulkanDevice::device;
}
rfct::VulkanDevice::VulkanDevice()
{
	RFCT_LOGGER_INIT();
	RFCT_INFO("Vulkan device created {}", "successfully");
		/*
		vk::ApplicationInfo appInfo("Reflect", VK_MAKE_VERSION(1, 0, 0), "Reflect engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);
		vk::InstanceCreateInfo instanceCreateInfo({}, &appInfo);
		m_Instance = vk::createInstanceUnique(instanceCreateInfo);
		std::vector<vk::PhysicalDevice> physicalDevices = m_Instance->enumeratePhysicalDevices();
		m_PhysicalDevice = physicalDevices[0];
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_PhysicalDevice.getQueueFamilyProperties();
		uint32_t queueFamilyIndex = 0;
		for (const auto& queueFamilyProperty : queueFamilyProperties) {
			if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics) {
				m_QueueFamilyIndex = queueFamilyIndex;
				break;
			}
			queueFamilyIndex++;
		}
		float queuePriority = 1.0f;
		vk::DeviceQueueCreateInfo deviceQueueCreateInfo({}, m_QueueFamilyIndex, 1, &queuePriority);
		vk::DeviceCreateInfo deviceCreateInfo({}, 1, &deviceQueueCreateInfo);
		m_Device = m_PhysicalDevice.createDeviceUnique(deviceCreateInfo);
		m_Queue = m_Device->getQueue(m_QueueFamilyIndex, 0);*/
}