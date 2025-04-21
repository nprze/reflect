
#include "android_window.h"

rfct::AndroidWindow::AndroidWindow(ANativeWindow* nativeWindow) {
    create(nativeWindow);
}

void rfct::AndroidWindow::create(ANativeWindow* nativeWindow) {
    if (!nativeWindow) {
        RFCT_CRITICAL("Failed to get a valid ANativeWindow");
    }
    window = nativeWindow;
    RFCT_TRACE("creating window: {}", (uint64_t)&window);
    extent = vk::Extent2D(ANativeWindow_getWidth(window), ANativeWindow_getHeight(window));
    //RFCT_TRACE("Width: {}", extent.width);
    //RFCT_TRACE("Height: {}", extent.height);
}

void rfct::AndroidWindow::destroyWind() {
    RFCT_TRACE("desctroying window: {}", (uint64_t)&window);
    if (window) {
        ANativeWindow_release(window);
        window = nullptr;
    }
}

vk::SurfaceKHR rfct::AndroidWindow::createSurface(vk::Instance instance) {
    VkSurfaceKHR surface;
    VkAndroidSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.window = window;
    VkResult res = vkCreateAndroidSurfaceKHR(instance, &createInfo, nullptr, &surface);
    if (res != VK_SUCCESS) {
        RFCT_CRITICAL("Failed to create Vulkan surface");
    }
    return vk::SurfaceKHR(surface);
}
