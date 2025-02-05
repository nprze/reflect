#pragma once
namespace rfct {
	inline std::vector<const char*> VulkanInstanceExtensions{
#ifndef RFCT_VULKAN_DEBUG_OFF
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME 
#endif // RFCT_VULKAN_DEBUG_OFF

	};

	class vulkanInstance {
	public:
		vk::Instance getInstance() { return m_instance.get(); }
		vk::SurfaceKHR getSurface() { return m_surface.get(); }
		vulkanInstance();
	private:
		vk::UniqueInstance m_instance;
		vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic> m_debugMessenger;
		vk::DispatchLoaderDynamic m_dynamicLoader;
		vk::UniqueSurfaceKHR m_surface;
	};
}