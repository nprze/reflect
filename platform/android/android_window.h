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
        ~AndroidWindow() { }
        inline float getAspectRatio() { return (float)((float)(extent.width) / (float)(extent.height)); }
        void create(ANativeWindow* nativeWindow);
        void create(const char* title, bool maximized = true, int width = 0, int height = 0) override {};
        void destroyWind();
        void destroy() override {};
        void show() override {}
        void hide() override {}
        vk::SurfaceKHR createSurface(vk::Instance instance) override;
        inline void setExtent(vk::Extent2D ext) override { extent = ext; }
        bool pollAndParseEvents() override { return true; };
        inline vk::Extent2D getExtent() override { return extent; }
        ANativeWindow* GetHandle() const { return window; }
    private:
        vk::Extent2D extent;
        ANativeWindow* window = nullptr;
        friend class input;
    };
}
