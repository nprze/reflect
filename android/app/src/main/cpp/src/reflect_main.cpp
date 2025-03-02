/*#ifdef VLD_ENABLE
#include <vld.h>
#endif // VLD_ENABLE
#include <iostream>
#include "app.h"
using namespace rfct;

int main() {
	reflectApplication(nullptr);
	return 0;
}*/
#include <jni.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include "app.h"
#include <iostream>
extern "C" JNIEXPORT void JNICALL
Java_reflect_mobile_reflect_MainActivity_createVulkanSurface(JNIEnv *env, jobject thiz, jobject surface) {
    std::cout<<"meow meow";

ANativeWindow* nativeWindow = ANativeWindow_fromSurface(env, surface);
rfct::reflectApplication app(nativeWindow);
}
