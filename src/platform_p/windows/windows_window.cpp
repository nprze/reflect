
#include "windows_window.h"
#include <stdexcept>

rfct::GlfwWindow::GlfwWindow(int width, int height, const char* title) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    create(width, height, title);
    inputLayer = std::make_unique<windowsInputLayer>();
}

void rfct::GlfwWindow::create(int width, int height, const char* title) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        throw std::runtime_error("Failed to create GLFW window");
    }
    extent = vk::Extent2D(width, height);
}

void rfct::GlfwWindow::destroy() {
    if (window) {
        glfwDestroyWindow(window);
        glfwTerminate();
        window = nullptr;
    }
}

void rfct::GlfwWindow::show() {
    glfwShowWindow(window);
}

void rfct::GlfwWindow::hide() {
    glfwHideWindow(window);
}

bool rfct::GlfwWindow::pollEvents() {
    glfwPollEvents();
    return !glfwWindowShouldClose(window);
}

vk::SurfaceKHR rfct::GlfwWindow::createSurface(vk::Instance instance) {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan surface");
    }
    return vk::SurfaceKHR(surface);
}
