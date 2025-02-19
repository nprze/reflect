#include "vulkan_instance.h"
#include "renderer_p\renderer.h"


VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    vk::DebugUtilsMessageTypeFlagsEXT messageType,
    const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    switch (messageSeverity)
    {
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

rfct::vulkanInstance::vulkanInstance()
{
	RFCT_PROFILE_FUNCTION();
    try {
        vk::ApplicationInfo appInfo(
            "game",
            VK_MAKE_VERSION(1, 0, 0),
            "reflect",
            VK_MAKE_VERSION(1, 0, 0),
            VK_API_VERSION_1_2
        );

        auto extensions = vk::enumerateInstanceExtensionProperties();
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
#ifndef RFCT_VULKAN_DEBUG_OFF
				RFCT_CRITICAL("Validation layer requested, but not avaible");
#endif // !RFCT_VULKAN_DEBUG_OFF
            }
        }
        
        vk::InstanceCreateInfo createInfo(
            {},
            &appInfo,0,nullptr,
            //validationLayers.size(),
            //validationLayers.data(),
            extensionNames.size(),
            extensionNames.data()
        );

        m_instance = vk::createInstanceUnique(createInfo);
#ifndef RFCT_VULKAN_DEBUG_OFF
        m_dynamicLoader = vk::DispatchLoaderDynamic(*m_instance, vkGetInstanceProcAddr);


        vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo(
            {},
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(debugCallback)
        );


        m_debugMessenger = m_instance.get().createDebugUtilsMessengerEXTUnique(debugCreateInfo, nullptr, m_dynamicLoader);
#endif // !RFCT_VULKAN_DEBUG_OFF

        m_surface = vk::UniqueSurfaceKHR(renderer::ren.getWindow().createSurface(m_instance.get()), m_instance.get());

		RFCT_TRACE("Vulkan instance created successfully");

    }
    catch (const vk::SystemError& e) {
		RFCT_ERROR("vk::SystemError: {}", e.what());
    }
}

