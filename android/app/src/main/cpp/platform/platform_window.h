#pragma once
#ifdef ANDROID_BUILD
#include "android\android_window.h"
#include <android/native_window.h>
#define RFCT_PLATFORM_WINDOW AndroidWindow
#define RFCT_NATIVE_WINDOW_ANDROID ANativeWindow*
#define RFCT_NATIVE_WINDOW_ANDROID_VAR nativeWidnowPtr
#define RFCT_WINDOWS_WINDOW_ARGUMENTS
#else
#include "windows\glfw_window.h"
#define RFCT_PLATFORM_WINDOW GlfwWindow
#define RFCT_NATIVE_WINDOW_ANDROID
#define RFCT_NATIVE_WINDOW_ANDROID_VAR
#define RFCT_WINDOWS_WINDOW_ARGUMENTS 968, 422, "reflect"
#endif // ANDROID_BUILD
