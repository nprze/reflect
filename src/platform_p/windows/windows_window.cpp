#include "windows_window.h"

rfct::windowsWindow::windowsWindow(int width, int height, const char* title)
{
	create(width, height, title);
	inputLayer = std::make_unique<windowsInputLayer>();
}

void rfct::windowsWindow::create(int width, int height, const char* title) {

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = "reflectWindow";
    RegisterClass(&wc);

    hInstance = GetModuleHandle(nullptr);

    hwnd = CreateWindow(
        "reflectWindow", title,
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        width, height, nullptr, nullptr, hInstance, nullptr);
	RFCT_TRACE("Window ({},{}) created successfully", width, height);
}

void rfct::windowsWindow::destroy()
{
    if (hwnd) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}

void rfct::windowsWindow::show()
{
    ShowWindow(hwnd, SW_SHOW);
}

void rfct::windowsWindow::hide()
{
    ShowWindow(hwnd, SW_HIDE);
}

bool rfct::windowsWindow::pollEvents()
{
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

LRESULT CALLBACK rfct::windowsWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}