#pragma once
#include "platform_window.h"
namespace rfct {
	inline std::vector<const char*> VulkanInstanceExtensions{
#ifndef RFCT_VULKAN_DEBUG_OFF
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif // RFCT_VULKAN_DEBUG_OFF
        VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
	};

    class vulkanInstance {
    public:
        vk::Instance getInstance() { return m_instance.get(); }
        vk::SurfaceKHR getSurface() { return m_surface.get(); }
        RFCT_ANDROID_VULKAN_INSTANCE_NAMESPACE DispatchLoaderDynamic& getDynamicLoader() { return m_dynamicLoader; }
        vulkanInstance();
    private:
        vk::UniqueInstance m_instance;
        vk::UniqueHandle<vk::DebugUtilsMessengerEXT,RFCT_ANDROID_VULKAN_INSTANCE_NAMESPACE DispatchLoaderDynamic> m_debugMessenger;
        RFCT_ANDROID_VULKAN_INSTANCE_NAMESPACE DispatchLoaderDynamic m_dynamicLoader;
        vk::UniqueSurfaceKHR m_surface;
    };
}