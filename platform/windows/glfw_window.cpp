#include "glfw_window.h"
#include <stdexcept>
#include "renderer_p\renderer.h"
#include "app.h"

void framebufferResizeCallback(GLFWwindow* window, int width, int height) {

	rfct::renderer::getRen().getDeviceWrapper().getSwapChain().framebufferResized = true;
    if (width == 0 && height == 0)
    {
        rfct::reflectApplication::shouldRender = false;
    }
    else {
        rfct::reflectApplication::shouldRender = true;
    }
}

rfct::GlfwWindow::GlfwWindow(int width, int height, const char* title) {
    if (!glfwInit()) {
        RFCT_CRITICAL("Failed to initialize GLFW");
    }
    create(width, height, title);
}

void rfct::GlfwWindow::create(int width, int height, const char* title) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        RFCT_CRITICAL("Failed to create GLFW window");
    }
    extent = vk::Extent2D(width, height);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
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
    return !glfwWindowShouldClose(window);
}

vk::SurfaceKHR rfct::GlfwWindow::createSurface(vk::Instance instance) {
    VkSurfaceKHR surface;
    VkResult res = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    if (res != VK_SUCCESS) {
        RFCT_CRITICAL("Failed to create Vulkan surface");
    }
    return vk::SurfaceKHR(surface);
}
