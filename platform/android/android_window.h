#pragma once

#include <vulkan/vulkan.hpp>
#include <android/native_window.h>
#include <android/native_activity.h>
#include "window.h"

namespace rfct {
    class AndroidWindow : public windowAbstact {
    public:
        AndroidWindow() = delete;
        AndroidWindow(ANativeWindow* nativeWindow);
        ~AndroidWindow() { destroy(); }

        void create(ANativeWindow* nativeWindow);
        void create(int width, int height, const char* title) override {};
        void destroy() override;
        void show() override {}
        void hide() override {}
        bool pollEvents() override; // TODO: Needs integration with an event system
        inline vk::Extent2D getExtent() override { return extent; }
        vk::SurfaceKHR createSurface(vk::Instance instance) override;

        ANativeWindow* GetHandle() const { return window; }
    private:
        vk::Extent2D extent;
        ANativeWindow* window = nullptr;
    };
}
