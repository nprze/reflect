#include "android_window.h"
#ifdef ANDROID_BUILD
#include <android/native_window.h>
#include <android/native_activity.h>
#include <android/log.h>
#include <vulkan/vulkan.hpp>

    rfct::AndroidWindow::AndroidWindow(int width, int height, const char* title)
        : extent(width, height) {
        create(width, height, title);
    }

    void rfct::AndroidWindow::create(int width, int height, const char* title) {
        extent = vk::Extent2D(width, height);

        ANativeWindow* nativeWindow = nullptr;

        if (!nativeWindow) {
            __android_log_print(ANDROID_LOG_ERROR, "AndroidWindow", "Native window creation failed!");
            return;
        }

        surface = createSurface(instance);
    }

    void rfct::AndroidWindow::destroy() {
        if (surface) {
            instance.destroySurfaceKHR(surface);
            surface = nullptr;
        }
    }

    void rfct::AndroidWindow::show() {
        __android_log_print(ANDROID_LOG_INFO, "AndroidWindow", "Window shown.");
    }

    void rfct::AndroidWindow::hide() {
        __android_log_print(ANDROID_LOG_INFO, "AndroidWindow", "Window hidden.");
    }

    bool rfct::AndroidWindow::pollEvents() {
        return true;
    }

    vk::SurfaceKHR rfct::AndroidWindow::createSurface(vk::Instance instance) {
        VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.window = nativeWindow; 

        VkSurfaceKHR nativeSurface;
        VkResult result = vkCreateAndroidSurfaceKHR(static_cast<VkInstance>(instance), &surfaceCreateInfo, nullptr, &nativeSurface);
        if (result != VK_SUCCESS) {
            __android_log_print(ANDROID_LOG_ERROR, "AndroidWindow", "Failed to create Vulkan surface!");
            return nullptr;
        }

        return vk::SurfaceKHR(nativeSurface);
    }


#endif

    rfct::AndroidWindow::AndroidWindow(int width, int height, const char* title)
        : extent(width, height) {
        create(width, height, title);
    }

    void rfct::AndroidWindow::create(int width, int height, const char* title) {
    }

    void rfct::AndroidWindow::destroy() {
    }

    void rfct::AndroidWindow::show() {
    }

    void rfct::AndroidWindow::hide() {
    }

    bool rfct::AndroidWindow::pollEvents() {
        return true;
    }

    vk::SurfaceKHR rfct::AndroidWindow::createSurface(vk::Instance instance) {
        return vk::SurfaceKHR(nullptr);
    }