#pragma once
#include <windows.h>
#include <string>
#include "platform_p/window.h"
#include "platform_p/windows/windows_input_layer.h"
namespace rfct {
    class windowsWindow : public windowAbstact {
    public:
        windowsWindow() = delete;
        windowsWindow(int width, int height, const char* title);
        ~windowsWindow() { destroy(); };
        void create(int width, int height, const char* title) override;
        void destroy() override;
        void show() override;
        void hide() override;
        bool pollEvents() override;
        inline vk::Extent2D getExtent()  override { return extent; };
        vk::SurfaceKHR createSurface(vk::Instance instance) override;

        inputLayer* getInputLayer() override { return inputLayer.get(); }
        HWND GetHandle() const { return hwnd; }
        HINSTANCE GetInstance() const { return hInstance; }
    private:
        vk::Extent2D extent;
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        unique<windowsInputLayer> inputLayer;
        HWND hwnd = nullptr;
        HINSTANCE hInstance = nullptr;
    };
}