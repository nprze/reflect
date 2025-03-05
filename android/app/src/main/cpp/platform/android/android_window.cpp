
#include "android_window.h"

rfct::AndroidWindow::AndroidWindow(ANativeWindow* nativeWindow) {
    create(nativeWindow);
}

void rfct::AndroidWindow::create(ANativeWindow* nativeWindow) {
    if (!nativeWindow) {
        RFCT_CRITICAL("Failed to get a valid ANativeWindow");
    }
    window = nativeWindow;
    extent = vk::Extent2D(ANativeWindow_getWidth(window), ANativeWindow_getHeight(window));
    RFCT_TRACE("Width: {}", extent.width);
    RFCT_TRACE("Height: {}", extent.height);
}

void rfct::AndroidWindow::destroy() {
    if (window) {
        window = nullptr;
    }
}

bool rfct::AndroidWindow::pollEvents() {
    return true;
}

vk::SurfaceKHR rfct::AndroidWindow::createSurface(vk::Instance instance) {
    VkSurfaceKHR surface;
    VkAndroidSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.window = window;

    if (vkCreateAndroidSurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
        RFCT_CRITICAL("Failed to create Vulkan surface");
    }

    return vk::SurfaceKHR(surface);
}
