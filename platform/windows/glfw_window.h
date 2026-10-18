#pragma once
#ifdef WIN32
#include <GLFW/glfw3.h>
#endif // WIN32
#include "window.h"

namespace rfct {
    class GlfwWindow : public RfctWindowAbstact {
    public:
        inline vk::Extent2D GetExtent() override { return m_extent; }
		inline float GetAspectRatio() { return (float)((float)(m_extent.width) / (float)(m_extent.height)); }
        GLFWwindow* GetHandle() const { return m_window; }
        inline void SetExtent(vk::Extent2D ext) override { m_extent = ext; }
    public:
        GlfwWindow() = delete;
        GlfwWindow(const char* title, bool maximized = false, int width = 600, int height = 600);
        ~GlfwWindow() { Destroy(); }
        void Create(const char* title, bool maximized = true, int width = 0, int height = 0) override;
        void Destroy() override;
        void Show() override;
        void Hide() override;
        bool PollAndParseEvents() override;
        vk::SurfaceKHR CreateSurface(vk::Instance instance) override;
    private:
        vk::Extent2D m_extent;
        GLFWwindow* m_window = nullptr;

    friend class input;
    };
}