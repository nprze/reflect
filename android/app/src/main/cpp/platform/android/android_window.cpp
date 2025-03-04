
#include "android_window.h"
#include <android/log.h>

rfct::AndroidWindow::AndroidWindow(ANativeWindow* nativeWindow) {
    create(nativeWindow);
}

#define LOG_TAG "reflect"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
void rfct::AndroidWindow::create(ANativeWindow* nativeWindow) {
    if (!nativeWindow) {
        RFCT_CRITICAL("Failed to get a valid ANativeWindow");
    }
    window = nativeWindow;
    extent = vk::Extent2D(ANativeWindow_getWidth(window), ANativeWindow_getHeight(window));
    LOGI("Width: %d", extent.width);
    LOGI("Height: %d", extent.height);
}

void rfct::AndroidWindow::destroy() {
    if (window) {
        window = nullptr;
    }
}

bool rfct::AndroidWindow::pollEvents() {
    // Android handles events differently; typically done in the main loop
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
