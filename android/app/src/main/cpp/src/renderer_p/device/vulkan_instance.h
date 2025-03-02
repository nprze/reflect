#pragma once
namespace rfct {
	inline std::vector<const char*> VulkanInstanceExtensions{
#ifndef RFCT_VULKAN_DEBUG_OFF
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif // RFCT_VULKAN_DEBUG_OFF
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
	};

	class vulkanInstance {
	public:
		vk::Instance getInstance() { return m_instance.get(); }
		vk::SurfaceKHR getSurface() { return m_surface.get(); }
		vk::detail::DispatchLoaderDynamic& getDynamicLoader() { return m_dynamicLoader; }
		vulkanInstance();
	private:
		vk::UniqueInstance m_instance;
		vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::detail::DispatchLoaderDynamic> m_debugMessenger;
		vk::detail::DispatchLoaderDynamic m_dynamicLoader;
		vk::UniqueSurfaceKHR m_surface;
	};
}