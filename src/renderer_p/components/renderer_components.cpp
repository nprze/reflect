#include "renderer_components.h"
#include "renderer_p\renderer.h"
#include "renderer_p\queues\vulkan_queue.h"
#include <set>

// requirements
const std::array<const char*, 1> deviceRequiredExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const std::vector<const char*> vulkanInstanceExtensions = {
#ifndef RFCT_VULKAN_DEBUG_OFF
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif // RFCT_VULKAN_DEBUG_OFF
		VK_KHR_SURFACE_EXTENSION_NAME
};

// helper functions
std::string queueFlagsToString(vk::QueueFlags queueFlags) {
	std::vector<std::string> flagNames;

	if (queueFlags & vk::QueueFlagBits::eGraphics) {
		flagNames.push_back("Graphics");
	}
	if (queueFlags & vk::QueueFlagBits::eCompute) {
		flagNames.push_back("Compute");
	}
	if (queueFlags & vk::QueueFlagBits::eTransfer) {
		flagNames.push_back("Transfer");
	}
	if (queueFlags & vk::QueueFlagBits::eSparseBinding) {
		flagNames.push_back("Sparse Binding");
	}
	if (queueFlags & vk::QueueFlagBits::eProtected) {
		flagNames.push_back("Protected");
	}

	if (flagNames.empty()) {
		return "{}";
	}

	std::ostringstream oss;
	oss << "{ ";
	for (size_t i = 0; i < flagNames.size(); ++i) {
		oss << flagNames[i];
		if (i != flagNames.size() - 1) {
			oss << " | ";
		}
	}
	oss << " }";

	return oss.str();
}

uint32_t selectQueueFamily(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
	RFCT_PROFILE_FUNCTION();
	std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

	std::pair<int, uint32_t> graphicsAndPresentFamily = { -1, 0 };
	std::pair<int, uint32_t> computeFamily = { -1, 0 };
	std::pair<int, uint32_t> transferFamily = { -1, 0 };

	// give priority to queue families with more queue count
	for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
		uint32_t queueCount = queueFamilies[i].queueCount;
		vk::QueueFlags flags = queueFamilies[i].queueFlags;
		RFCT_INFO("queue {}, flags {}", i, queueFlagsToString(flags));
		bool supportsPresent = physicalDevice.getSurfaceSupportKHR(i, surface);

		if ((flags & vk::QueueFlagBits::eGraphics) && supportsPresent) {
			if (queueCount > (graphicsAndPresentFamily.second)) {
				graphicsAndPresentFamily = { i, queueCount };
			}
		}
		else if ((flags & vk::QueueFlagBits::eCompute) && !(flags & vk::QueueFlagBits::eGraphics)) {
			if (queueCount > (computeFamily.second)) {
				computeFamily = { i, queueCount };
			}
		}
		else if ((flags & vk::QueueFlagBits::eTransfer) && !(flags & vk::QueueFlagBits::eGraphics) && !(flags & vk::QueueFlagBits::eCompute)) {
			if (queueCount > (transferFamily.second)) {
				transferFamily = { i, queueCount };
			}
		}
	}
	return static_cast<uint32_t>(graphicsAndPresentFamily.first);
}

uint32_t rateDevice(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
	RFCT_PROFILE_FUNCTION();
	uint32_t queueFamily = selectQueueFamily(device, surface);
	if (queueFamily == -1) {
        RFCT_ERROR("not enough queue families");
		return 0;
	}

	vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
	vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();

	// ensure required extensions are supported
	std::vector<vk::ExtensionProperties> extensions = device.enumerateDeviceExtensionProperties();
	std::vector<const char*> requiredExtensions(deviceRequiredExtensions.begin(), deviceRequiredExtensions.end() );
	for (const auto& ext : extensions) {
		std::string extName = ext.extensionName.data();
		auto it = std::find(requiredExtensions.begin(), requiredExtensions.end(), extName);
		if (it != requiredExtensions.end()) {
			requiredExtensions.erase(it);
		}
	}

	if (!requiredExtensions.empty()) {
        RFCT_ERROR("required extensions not supported");
		return 0;
	}

	uint32_t score = 100;

	// Prefer discrete GPUs
	if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
		score += 500;
	}
	// Prefer higher compute performance
	score += deviceProperties.limits.maxComputeSharedMemorySize / 1024;

	return score;
}

vk::PhysicalDevice ChooseBestPhysicalDevice() {
	RFCT_PROFILE_FUNCTION();
	std::vector<vk::PhysicalDevice> physicalDevices = renderer::getRen().getInstance().enumeratePhysicalDevices();
	std::vector<uint32_t> ratings;
	for (uint32_t i = 0; i < physicalDevices.size(); i++) {
		ratings.push_back(rateDevice(physicalDevices[i]));
		std::string deviceNameStr = physicalDevices[i].getProperties().deviceName;
		RFCT_TRACE("Physical device {} rated {}", deviceNameStr, ratings[i]);
	}
	uint32_t bestRating = 0;
	uint32_t bestDeviceIndex = -1;
	for (uint32_t i = 0; i < ratings.size(); i++) {
		if (ratings[i] > bestRating) {
			bestDeviceIndex = i;
			bestRating = ratings[i];
		}
	}
	if (bestDeviceIndex != -1) {
		return physicalDevices[bestDeviceIndex];
	}
	else {
		RFCT_CRITICAL("failed to find suitable physical device");
	}
}

vk::UniqueDevice createDevice(vk::PhysicalDevice physicalDevice) {
	RFCT_PROFILE_FUNCTION();
	uint32_t queueFamilies = selectQueueFamily(physicalDevice);
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	float queuePriority = 1.0f;
	vk::DeviceQueueCreateInfo graphicsQueueCreateInfo = {};
	graphicsQueueCreateInfo.queueFamilyIndex = queueFamilies;
	graphicsQueueCreateInfo.queueCount = 1;
	graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;
	queueCreateInfos.push_back(graphicsQueueCreateInfo);

	vk::PhysicalDeviceFeatures deviceFeatures = {};

	vk::DeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(RfctDevice::deviceRequiredExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = RfctDevice::deviceRequiredExtensions.data();

	vk::UniqueDevice device;
	try {
		device = physicalDevice.createDeviceUnique(deviceCreateInfo);
	}
	catch (const std::exception& e) {
		RFCT_CRITICAL("failed to create device");
	}
	return device;
}

rfct::RfctDevice::RfctDevice()
	: m_physicalDevice(ChooseBestPhysicalDevice()), 
	m_device(createDevice(m_physicalDevice)), 
	m_queueManager(m_device.get(), m_physicalDevice) {
	std::string deviceNameStr = m_physicalDevice.getProperties().deviceName;
	RFCT_TRACE("Physical device chosen: {}", deviceNameStr);
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
	vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	vk::DebugUtilsMessageTypeFlagsEXT messageType,
	const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	switch (messageSeverity) {
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
		RFCT_TRACE("Validation Layer: {}", pCallbackData->pMessage);
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
		RFCT_INFO("Validation Layer: {}", pCallbackData->pMessage);
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
		RFCT_WARN("Validation Layer: {}", pCallbackData->pMessage);
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
		RFCT_ERROR("Validation Layer: {}", pCallbackData->pMessage);
		break;
	default:
		break;
	}
	return VK_FALSE;
}

rfct::RfctVulkanInstance::RfctVulkanInstance() {
	RFCT_PROFILE_FUNCTION();
	try {
		vk::ApplicationInfo appInfo(
			"smokes",
			VK_MAKE_VERSION(1, 0, 0),
			"reflect",
			VK_MAKE_VERSION(1, 0, 0),
			VK_API_VERSION_1_2
		);

		std::vector<vk::ExtensionProperties>  extensions = vk::enumerateInstanceExtensionProperties();
		std::vector<const char*> extensionNames;
		for (const auto& ext : extensions) {
			extensionNames.push_back(ext.extensionName);
		}
		std::vector<vk::ExtensionProperties> availableExtensions = vk::enumerateInstanceExtensionProperties();

		for (const char* ext : VulkanInstanceExtensions) {
			bool found = false;
			for (const auto& available : availableExtensions) {
				if (strcmp(available.extensionName, ext) == 0) {
					found = true;
					break;
				}
			}
			if (!found) {
				RFCT_CRITICAL("VulkanInstanceExtension {} not avaible", ext);
			}
		}

		std::vector<const char*> validationLayers = {
#ifndef RFCT_VULKAN_DEBUG_OFF
			"VK_LAYER_KHRONOS_validation"
#endif // !RFCT_VULKAN_DEBUG_OFF
		};
		std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();
		for (const char* layerName : validationLayers) {
			bool layerFound = false;
			for (const auto& layer : availableLayers) {
				if (strcmp(layer.layerName, layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				validationLayers.clear();
				RFCT_CRITICAL("Validation layer requested, but not avaible");
			}
		}

		vk::InstanceCreateInfo createInfo(
			{},
			&appInfo,
#ifndef RFCT_VULKAN_DEBUG_OFF
			validationLayers.size(),
			validationLayers.data(),
#else
			0, nullptr,
#endif // !RFCT_VULKAN_DEBUG_OFF
			extensionNames.size(),
			extensionNames.data()
		);

		m_instance = vk::createInstanceUnique(createInfo);

#ifndef RFCT_VULKAN_DEBUG_OFF
		m_dynamicLoader = RFCT_VULKAN_INSTANCE_NAMESPACE DispatchLoaderDynamic(*m_instance, vkGetInstanceProcAddr);


		vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo(
			{},
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
			reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(debugCallback)
		);
		m_debugMessenger = m_instance.get().createDebugUtilsMessengerEXTUnique(debugCreateInfo, nullptr, m_dynamicLoader);
#endif // !RFCT_VULKAN_DEBUG_OFF
	}
	catch (const vk::SystemError& e) {
		RFCT_ERROR("vk::SystemError: {}", e.what());
	}
}

