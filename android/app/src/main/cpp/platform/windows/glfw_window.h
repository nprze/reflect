#pragma once
#ifdef WIN32
#include <GLFW/glfw3.h>
#endif // WIN32

#include <vulkan/vulkan.hpp>
#include <memory>
#include "window.h"

namespace rfct {
    class GlfwWindow : public windowAbstact {
    public:
        GlfwWindow() = delete;
        GlfwWindow(int width, int height, const char* title);
        ~GlfwWindow() { destroy(); }

        void create(int width, int height, const char* title) override;
        void destroy() override;
        void show() override;
        void hide() override;
        bool pollEvents() override;
        inline vk::Extent2D getExtent() override { return extent; }
        vk::SurfaceKHR createSurface(vk::Instance instance) override;

        GLFWwindow* GetHandle() const { return window; }
    private:
        vk::Extent2D extent;
        GLFWwindow* window = nullptr;
    };
}