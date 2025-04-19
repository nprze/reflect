#pragma once
#ifdef ANDROID_BUILD
#include "android\android_window.h"
#define RFCT_PLATFORM_WINDOW AndroidWindow
#define RFCT_APP_ARGS ANativeWindow* nativeWidnowPtr
#define RFCT_NATIVE_WINDOW_ANDROID ANativeWindow*
#define RFCT_NATIVE_WINDOW_ANDROID_VAR nativeWidnowPtr
#define RFCT_WINDOWS_WINDOW_ARGUMENTS
#define RFCT_ANDROID_VULKAN_INSTANCE_NAMESPACE vk::detail::
#define RFCT_RENDERER_ARGUMENTS RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR,rfct::AssetsManager* assetsManager
#define RFCT_RENDERER_ARGUMENTS_VAR RFCT_NATIVE_WINDOW_ANDROID_VAR, &m_AssetsManager
#else
#include "windows\glfw_window.h"
#define RFCT_PLATFORM_WINDOW GlfwWindow
#define RFCT_APP_ARGS
#define RFCT_NATIVE_WINDOW_ANDROID
#define RFCT_NATIVE_WINDOW_ANDROID_VAR
#define RFCT_WINDOWS_WINDOW_ARGUMENTS 968, 422, "reflect"
#define RFCT_ANDROID_VULKAN_INSTANCE_NAMESPACE vk::
#define RFCT_RENDERER_ARGUMENTS rfct::AssetsManager* assetsManager
#define RFCT_RENDERER_ARGUMENTS_VAR &m_AssetsManager
#endif // ANDROID_BUILD
