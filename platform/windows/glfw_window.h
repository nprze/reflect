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
        GlfwWindow(const char* title, bool maximized = true, int width = 0, int height = 0);
        ~GlfwWindow() { destroy(); }
		inline float getAspectRatio() { return (float)((float)(extent.width) / (float)(extent.height)); }
        void create(const char* title, bool maximized = true, int width = 0, int height = 0) override;
        void destroy() override;
        void show() override;
        void hide() override;
        bool pollAndParseEvents() override;
        inline vk::Extent2D getExtent() override { return extent; }
        inline void setExtent(vk::Extent2D ext) override { extent = ext; }
        vk::SurfaceKHR createSurface(vk::Instance instance) override;

        GLFWwindow* GetHandle() const { return window; }
    private:
        vk::Extent2D extent;
        GLFWwindow* window = nullptr;
        friend class input;
    };
}