#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <memory>
#include "platform_p/window.h"
#include "platform_p/windows/windows_input_layer.h"

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

        inputLayer* getInputLayer() override { return inputLayer.get(); }
        GLFWwindow* GetHandle() const { return window; }
    private:
        vk::Extent2D extent;
        std::unique_ptr<windowsInputLayer> inputLayer;
        GLFWwindow* window = nullptr;
    };
}