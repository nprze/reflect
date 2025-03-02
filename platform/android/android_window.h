#pragma once
#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>
#include "window.h"

namespace rfct {
    class AndroidWindow : public windowAbstact {
    public:
        AndroidWindow() = delete;
        AndroidWindow(int width, int height, const char* title);
        ~AndroidWindow() { destroy(); }

        void create(int width, int height, const char* title) override;
        void destroy() override;
        void show() override;
        void hide() override;
        bool pollEvents() override;
        inline vk::Extent2D getExtent() override { return extent; }
        vk::SurfaceKHR createSurface(vk::Instance instance) override;

    private:
        vk::Extent2D extent;
    };
}