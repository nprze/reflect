#include "renderer_components.h"
#include <vma/vk_mem_alloc.h>
#include <set>
#include "assets/asset_manager.h"

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
std::string QueueFlagsToString(vk::QueueFlags queueFlags) {
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

uint32_t SelectQueueFamily(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, bool debugInfo = false) {
	RFCT_PROFILE_FUNCTION();
	std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

	std::pair<int, uint32_t> graphicsAndPresentFamily = { -1, 0 };
	std::pair<int, uint32_t> computeFamily = { -1, 0 };
	std::pair<int, uint32_t> transferFamily = { -1, 0 };

	// give priority to queue families with more queue count
	for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
		uint32_t queueCount = queueFamilies[i].queueCount;
		vk::QueueFlags flags = queueFamilies[i].queueFlags;
		if (debugInfo) RFCT_INFO("queue {}, flags {}", i, QueueFlagsToString(flags));
		bool supportsPresent = RFCT_VULKAN_SOFT_CHECK(physicalDevice.getSurfaceSupportKHR(i, surface));

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

uint32_t RateDevice(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
	RFCT_PROFILE_FUNCTION();
	uint32_t queueFamily = SelectQueueFamily(device, surface);
	if (queueFamily == -1) {
        RFCT_ERROR("not enough queue families");
		return 0;
	}

	vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
	vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();

	auto result = device.enumerateDeviceExtensionProperties();
	if (!RFCT_VULKAN_SOFT_CHECK(result)) return 0;

	// ensure required extensions are supported
	std::vector<vk::ExtensionProperties> extensions = result.value;
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

vk::PhysicalDevice ChooseBestPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface) {
	RFCT_PROFILE_FUNCTION();
	std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices().value;
	std::vector<uint32_t> ratings;
	for (uint32_t i = 0; i < physicalDevices.size(); i++) {
		ratings.push_back(RateDevice(physicalDevices[i], surface));
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

vk::UniqueDevice CreateDevice(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
	RFCT_PROFILE_FUNCTION();
	uint32_t queueFamilies = SelectQueueFamily(physicalDevice, surface);
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
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceRequiredExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceRequiredExtensions.data();

	vk::UniqueDevice device;
	auto createDeviceResult = physicalDevice.createDeviceUnique(deviceCreateInfo);
	RFCT_ASSERT(RFCT_VULKAN_SOFT_CHECK(createDeviceResult));
	device = std::move(createDeviceResult.value);
	return device;
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(
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

rfct::RfctDevice::RfctDevice(vk::Instance instance, vk::SurfaceKHR surface)
	: m_physicalDevice(ChooseBestPhysicalDevice(instance, surface)),
	m_device(CreateDevice(m_physicalDevice, surface)), 
	m_queue(m_device.get(), m_physicalDevice, surface) {
	std::string deviceNameStr = m_physicalDevice.getProperties().deviceName;
	RFCT_TRACE("Physical device chosen: {}", deviceNameStr);
}

rfct::RfctVulkanInstance::RfctVulkanInstance() {
	RFCT_PROFILE_FUNCTION();
	vk::ApplicationInfo appInfo(
		"smokes",
		VK_MAKE_VERSION(1, 0, 0),
		"reflect",
		VK_MAKE_VERSION(1, 0, 0),
		VK_API_VERSION_1_2
	);

	auto enumerateInstanceExtensionPropsResult = vk::enumerateInstanceExtensionProperties();
	RFCT_ASSERT(RFCT_VULKAN_SOFT_CHECK(enumerateInstanceExtensionPropsResult));

	std::vector<vk::ExtensionProperties> extensions = enumerateInstanceExtensionPropsResult.value;
	std::vector<const char*> extensionNames;
	for (const auto& ext : extensions) {
		extensionNames.push_back(ext.extensionName);
	}
	std::vector<vk::ExtensionProperties> availableExtensions = enumerateInstanceExtensionPropsResult.value;

	for (const char* ext : vulkanInstanceExtensions) {
		bool found = false;
		for (const auto& available : availableExtensions) {
			if (strcmp(available.extensionName, ext) == 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			RFCT_CRITICAL("VulkanInstanceExtension {} requested but not avaible", ext);
		}
	}

	std::vector<const char*> validationLayers = {
#ifndef RFCT_VULKAN_DEBUG_OFF
		"VK_LAYER_KHRONOS_validation"
#endif // !RFCT_VULKAN_DEBUG_OFF
	};

	auto enumerateInstanceLayerPropsResult = vk::enumerateInstanceLayerProperties();
	RFCT_ASSERT(RFCT_VULKAN_SOFT_CHECK(enumerateInstanceLayerPropsResult));

	std::vector<vk::LayerProperties> availableLayers = enumerateInstanceLayerPropsResult.value;
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

	auto instanceCreateResult = vk::createInstanceUnique(createInfo);
	RFCT_ASSERT(RFCT_VULKAN_SOFT_CHECK(instanceCreateResult));
	m_instance = std::move(instanceCreateResult.value);

#ifndef RFCT_VULKAN_DEBUG_OFF
	m_dynamicLoader = RFCT_VULKAN_LOADER_NAMESPACE::DispatchLoaderDynamic(*m_instance, vkGetInstanceProcAddr);

	vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo(
		{},
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
		reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(DebugCallback)
	);
	auto debugCreateResult = m_instance.get().createDebugUtilsMessengerEXTUnique(debugCreateInfo, nullptr, m_dynamicLoader);
	if (RFCT_VULKAN_SOFT_CHECK(debugCreateResult)) {
		m_debugMessenger = std::move(debugCreateResult.value);
		m_debugEnabled = true;
	}
#endif // !RFCT_VULKAN_DEBUG_OFF
}

rfct::RfctQueue::RfctQueue(vk::Device device, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
	: m_device(device) {
	RFCT_PROFILE_FUNCTION();
	m_graphicsQueueFamilyIndex = SelectQueueFamily(physicalDevice, surface, true);
	m_graphicsQueue = m_device.getQueue(m_graphicsQueueFamilyIndex, 0);
}

void rfct::RfctQueue::SubmitGraphics(const vk::SubmitInfo& submitInfo, vk::Fence fence) {
	RFCT_PROFILE_FUNCTION();
	m_graphicsQueue.submit(submitInfo, fence);
}

rfct::RfctSwapChain::RfctSwapChain(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface) {
	RFCT_PROFILE_FUNCTION();
	CreateSwapChain(physicalDevice, device, surface);
}

void rfct::RfctSwapChain::CreateSwapChain(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface) {
	RFCT_PROFILE_FUNCTION();
	// throw if not supported
	vk::SurfaceCapabilitiesKHR capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface).value;
	std::vector<vk::SurfaceFormatKHR> surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface).value;
	std::vector<vk::PresentModeKHR> presentModes = physicalDevice.getSurfacePresentModesKHR(surface).value;
	vk::SurfaceFormatKHR chosenSurfaceFormat = surfaceFormats[0];

	m_surfaceFormat = chosenSurfaceFormat;
	vk::PresentModeKHR  chosenPresentMode = vk::PresentModeKHR::eFifo;
#ifdef WINDOWS_BUILD
	for (vk::PresentModeKHR mode : presentModes) {
		if (mode == vk::PresentModeKHR::eMailbox)  chosenPresentMode = vk::PresentModeKHR::eMailbox;
	}
#endif // WINDOWS_BUILD
	RFCT_TRACE("Choosen swap chain present mode: {0}", chosenPresentMode == vk::PresentModeKHR::eMailbox ? "Mailbox" : "Fifo");
	m_swapChainExtent = capabilities.currentExtent;
	vk::SwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.surface = surface;
#ifdef WINDOWS_BUILD
	if (m_swapChain.get() != nullptr) {
		swapChainCreateInfo.oldSwapchain = m_swapChain.get();
	}
#endif // WINDOWS_BUILD

	if (capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eOpaque) {
		swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	}
#ifdef ANDROID_BUILD
	else if (capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit) {
		swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eInherit;
	}
	else {
		// Pick any available supported option
		swapChainCreateInfo.compositeAlpha = static_cast<vk::CompositeAlphaFlagBitsKHR>(
			__builtin_ctz(static_cast<uint32_t>(capabilities.supportedCompositeAlpha))
			);
	}
	vk::SurfaceTransformFlagBitsKHR transform = capabilities.currentTransform;
	switch (transform) {
	case vk::SurfaceTransformFlagBitsKHR::eRotate90:
		world::getWorld().addScreenTransform(90);
		break;
	case vk::SurfaceTransformFlagBitsKHR::eRotate180:
		world::getWorld().addScreenTransform(180);
		break;
	case vk::SurfaceTransformFlagBitsKHR::eRotate270:
		world::getWorld().addScreenTransform(270);
		break;
	default:
		break;
	}
#endif
	swapChainCreateInfo.minImageCount = RFCT_FRAMES_IN_FLIGHT + 1;
	swapChainCreateInfo.imageFormat = chosenSurfaceFormat.format;
	swapChainCreateInfo.imageColorSpace = chosenSurfaceFormat.colorSpace;
	swapChainCreateInfo.imageExtent = capabilities.currentExtent;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
	swapChainCreateInfo.preTransform = capabilities.currentTransform;
	swapChainCreateInfo.presentMode = chosenPresentMode;
	swapChainCreateInfo.clipped = VK_TRUE;

	auto swapChainCreateResult = device.createSwapchainKHRUnique(swapChainCreateInfo);
	RFCT_ASSERT(RFCT_VULKAN_SOFT_CHECK(swapChainCreateResult));
	m_swapChain = std::move(swapChainCreateResult.value);
}

void rfct::RfctSwapChain::RecreateSwapChain(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface) {
	RFCT_PROFILE_FUNCTION();
	device.waitIdle();
#ifdef ANDROID_BUILD
	device.destroySwapchainKHR(m_swapChain.get());
	*m_swapChain = nullptr;
#endif
	auto surfaceCapabilitiesQueryResult = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	if (!RFCT_VULKAN_SOFT_CHECK(surfaceCapabilitiesQueryResult)) return;
	vk::SurfaceCapabilitiesKHR capabilities = surfaceCapabilitiesQueryResult.value;
	if (capabilities.currentExtent.width == 0 || capabilities.currentExtent.height == 0) return;
	CreateSwapChain(physicalDevice, device, surface);
}

rfct::RfctSwapChain::RfctAcquireNextImageResult rfct::RfctSwapChain::AcquireNextImage(const vk::Semaphore& semaphore, vk::Fence fence, vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface) {
	RFCT_PROFILE_FUNCTION();
	rfct::RfctSwapChain::RfctAcquireNextImageResult result = {};
	result.internalResult = vk::Result::eErrorUnknown;
	result.imageIndex = -1;
	if (m_framebufferResized) {
		RecreateSwapChain(physicalDevice, device, surface);
		m_framebufferResized = false;
		result.needsRecreation = true;
		result.internalResult = vk::Result::eErrorOutOfDateKHR;
		return result;
	}
	auto acquireImageResult = device.acquireNextImageKHR(m_swapChain.get(), UINT64_MAX, semaphore, fence);
	result.internalResult = acquireImageResult.result;
	if (RFCT_VULKAN_SOFT_CHECK(acquireImageResult)) {
		result.internalResult = acquireImageResult.result;
		result.imageIndex = acquireImageResult.value;
		return result;
	}
	if (acquireImageResult.result == vk::Result::eErrorOutOfDateKHR) {
		RecreateSwapChain(physicalDevice, device, surface);
		result.needsRecreation = true;
		result.suboptimal = true;
		result.imageIndex = acquireImageResult.value;
		return result;
	}
	if (acquireImageResult.result == vk::Result::eSuboptimalKHR) {
		RFCT_WARN("Swap chain is suboptimal, recreating...");
		RecreateSwapChain(physicalDevice, device, surface);
		result.needsRecreation = true;
		result.suboptimal = true;
		result.imageIndex = acquireImageResult.value;
		return result;
	}
	RFCT_CRITICAL("unknown error ocurred when acquiring next image. Result code: {}", static_cast<uint32_t>(acquireImageResult.result));
}

rfct::RfctSurfaceWrapper::RfctSurfaceWrapper(vk::SurfaceKHR surfaceArg) {
	m_surface = surfaceArg;
}

void rfct::RfctSurfaceWrapper::NewSurface(vk::Instance instance, vk::SurfaceKHR surfaceArg) {
	instance.destroySurfaceKHR(m_surface);
	m_surface = surfaceArg;
}

void rfct::RfctSurfaceWrapper::DestroySurface(vk::Instance instance) {
	instance.destroySurfaceKHR(m_surface);
	m_surface = nullptr;
}

rfct::RfctVulkanMemAllocator::RfctVulkanMemAllocator(vk::PhysicalDevice physicalDevice, vk::Device device, vk::Instance instance) {
#ifdef ANDROID_BUILD
	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.physicalDevice = physicalDevice;
	allocatorCreateInfo.device = device;
	allocatorCreateInfo.instance = instance;
	allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
	VmaVulkanFunctions vulkanFunctions = {};

	vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
	vulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
	vulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	vulkanFunctions.vkCreateBuffer = vkCreateBuffer;
	allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

	vmaCreateAllocator(&allocatorCreateInfo, &m_allocator);
#else
	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.physicalDevice = physicalDevice;
	allocatorCreateInfo.device = device;
	allocatorCreateInfo.instance = instance;
	allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
	vmaCreateAllocator(&allocatorCreateInfo, &m_allocator);
#endif
}

rfct::RfctVulkanMemAllocator::~RfctVulkanMemAllocator() {
	vmaDestroyAllocator(m_allocator);
}
