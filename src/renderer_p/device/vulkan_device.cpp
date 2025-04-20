#include "vulkan_device.h"
#include "renderer_p\renderer.h"
#include "renderer_p\queues\vulkan_queue.h"
#include <set>

namespace rfct {
	 const std::array<const char*, 1> vulkanDevice::deviceRequiredExtensions = {
		//VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
		//VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		//VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
		//VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME
	};
	uint32_t rateDevice(vk::PhysicalDevice device) {
		std::array<uint32_t, 3> queueFamilies = selectQueueFamilies(device);
		for (uint32_t familyIndex : queueFamilies) {
			if (familyIndex == -1) {
				return 0;
			}
		}

		vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
		vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();

		// Ensure ray tracing extensions are supported
		std::vector<vk::ExtensionProperties> extensions = device.enumerateDeviceExtensionProperties();
		std::vector<const char*> requiredExtensions( vulkanDevice::deviceRequiredExtensions.begin(), vulkanDevice::deviceRequiredExtensions.end() );
		for (const auto& ext : extensions) {
			std::string extName = ext.extensionName.data();
			auto it = std::find(requiredExtensions.begin(), requiredExtensions.end(), extName);
			if (it != requiredExtensions.end()) {
				requiredExtensions.erase(it);
			}
		}

		if (!requiredExtensions.empty()) {
			return 0;
		}
/*
		// ray tracing support
		vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeatures{};
		vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelStructFeatures{};

		vk::PhysicalDeviceFeatures2 features2{};
		features2.pNext = &rtPipelineFeatures;
		rtPipelineFeatures.pNext = &accelStructFeatures;

		device.getFeatures2(&features2);

		if (!rtPipelineFeatures.rayTracingPipeline || !accelStructFeatures.accelerationStructure) {
			return 0;
		}*/

		uint32_t score = 100;

		// Prefer discrete GPUs
		if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
			score += 500;
		}
/*
		// Favor higher compute performance
		score += deviceProperties.limits.maxComputeSharedMemorySize / 1024;

		// Favor GPUs with more ray tracing capabilities
		score += rtPipelineFeatures.rayTracingPipelineTraceRaysIndirect ? 50 : 0;
		score += accelStructFeatures.accelerationStructureIndirectBuild ? 50 : 0;
*/
		return score;
	}

	vk::PhysicalDevice chooseBestPhysicalDevice()
	{
		RFCT_PROFILE_FUNCTION();
		std::vector<vk::PhysicalDevice> physicalDevices = renderer::getRen().getInstance().enumeratePhysicalDevices();
		std::vector<uint32_t> ratings;
		for (uint32_t i = 0; i < physicalDevices.size(); i++)
		{
			ratings.push_back(rateDevice(physicalDevices[i]));
			std::string deviceNameStr = physicalDevices[i].getProperties().deviceName;
			RFCT_TRACE("Physical device {} rated {}", deviceNameStr, ratings[i]);
		}
		uint32_t bestRating = 0;
		uint32_t bestDeviceIndex = -1;
		for (uint32_t i = 0; i < ratings.size(); i++)
		{
			if (ratings[i] > bestRating) {
				bestDeviceIndex = i;
				bestRating = ratings[i];
			}
		}
		if (bestDeviceIndex != -1)
		{
			return physicalDevices[bestDeviceIndex];
		}
		else
		{
			RFCT_CRITICAL("failed to find suitable physical device");
		}
	}

	vk::UniqueDevice createDevice(vk::PhysicalDevice physicalDevice) {
		RFCT_PROFILE_FUNCTION();
		
		std::array<uint32_t, 3> queueFamilies = selectQueueFamilies(physicalDevice);
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
		float queuePriority = 1.0f;
		vk::DeviceQueueCreateInfo graphicsQueueCreateInfo = {};
		graphicsQueueCreateInfo.queueFamilyIndex = queueFamilies[0];
		graphicsQueueCreateInfo.queueCount = 1;
		graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(graphicsQueueCreateInfo);

		if (queueFamilies[1] != queueFamilies[0]) {
			vk::DeviceQueueCreateInfo computeQueueCreateInfo = {};
			computeQueueCreateInfo.queueFamilyIndex = queueFamilies[1];
			computeQueueCreateInfo.queueCount = 1;
			computeQueueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(computeQueueCreateInfo);
		}

		if (queueFamilies[2] != queueFamilies[0] && queueFamilies[2] != queueFamilies[1]) {
			vk::DeviceQueueCreateInfo transferQueueCreateInfo = {};
			transferQueueCreateInfo.queueFamilyIndex = queueFamilies[2];
			transferQueueCreateInfo.queueCount = 1;
			transferQueueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(transferQueueCreateInfo);
		}

		vk::PhysicalDeviceFeatures deviceFeatures = {};

		vk::DeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(vulkanDevice::deviceRequiredExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = vulkanDevice::deviceRequiredExtensions.data();

		vk::UniqueDevice device;
		try {
			device = physicalDevice.createDeviceUnique(deviceCreateInfo);
		}
		catch (const std::exception& e) {
			RFCT_CRITICAL("failed to create device");
		}

		return device;
	}


	vulkanDevice::vulkanDevice() :
		m_physicalDevice(chooseBestPhysicalDevice()), 
		m_device(createDevice(m_physicalDevice)), 
		m_queueManager(m_device.get(), m_physicalDevice), 
		m_swapChain(m_device.get(), m_physicalDevice, renderer::getRen().getWindow().getExtent())
	{
		std::string deviceNameStr = m_physicalDevice.getProperties().deviceName;
		RFCT_TRACE("Physical device chosen: {}", deviceNameStr);
	}
}