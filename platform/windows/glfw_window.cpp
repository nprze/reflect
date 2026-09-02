#include "glfw_window.h"
#include <stdexcept>
#include "renderer_p/renderer.h"
#include "app.h"

void FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
	RFCT_PROFILE_FUNCTION();
	rfct::GetRen().GetSwapChain().framebufferResized = true;
    vk::Extent2D newExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    rfct::GetRen().GetWindow().SetExtent(newExtent);
    if (width == 0 && height == 0)
    {
        rfct::reflectApplication::isAppMinimised = true;
    }
    else {
        rfct::reflectApplication::isAppMinimised = false;
    }
}

rfct::GlfwWindow::GlfwWindow(const char* title, bool maximized, int width, int height) {
    RFCT_PROFILE_FUNCTION();
    if (!glfwInit()) {
        RFCT_CRITICAL("Failed to initialize GLFW");
    }
    Create(title, maximized, width, height);
}

void rfct::GlfwWindow::Create(const char* title, bool maximized, int width, int height) {
    RFCT_PROFILE_FUNCTION();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    if (maximized) {

        GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
        m_window = glfwCreateWindow(mode->width, mode->height, title, nullptr, nullptr);
        width = mode->width;
        height = mode->height;
        glfwMaximizeWindow(m_window);
        RFCT_ASSERT(m_window);
    }
    else {
        m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        RFCT_ASSERT(m_window);
    }
    m_extent = vk::Extent2D(width, height);
    glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);
}

void rfct::GlfwWindow::Destroy() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        glfwTerminate();
        m_window = nullptr;
    }
}

void rfct::GlfwWindow::Show() {
    glfwShowWindow(m_window);
}

void rfct::GlfwWindow::Hide() {
    glfwHideWindow(m_window);
}

bool rfct::GlfwWindow::PollAndParseEvents() {
    return !glfwWindowShouldClose(m_window);
}

vk::SurfaceKHR rfct::GlfwWindow::CreateSurface(vk::Instance instance) {
    RFCT_PROFILE_FUNCTION();
    VkSurfaceKHR surface;
    VkResult res = glfwCreateWindowSurface(instance, m_window, nullptr, &surface);
    if (res != VK_SUCCESS) {
        RFCT_CRITICAL("Failed to create Vulkan surface");
    }
    return vk::SurfaceKHR(surface);
}
