

#include <jni.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include "app.h"
#include <iostream>
#include "renderer_p/renderer.h"
#include <vector>
#include <android/log.h>
#include "android_glue.h"

void resizeCallback(int width, int height){
    rfct::renderer::getRen().getDeviceWrapper().getSwapChain().framebufferResized = true;
    if (width == 0 && height == 0)
    {
        rfct::reflectApplication::shouldRender = false;
    }
    else {
        rfct::reflectApplication::shouldRender = true;
    }

}

std::vector<rfct::InputEvent> rfct::InputQueue::eventQueue;

static std::unique_ptr<rfct::reflectApplication> app;
static jclass eventClass;
static jfieldID actionField;
static jfieldID xField;
static jfieldID yField;
static jfieldID timestampField;
extern "C" JNIEXPORT void JNICALL
Java_reflect_mobile_reflect_MainActivity_createVulkanApp(JNIEnv *env, jobject thiz, jobject surface) {
    if (!app) {  // Only create if it doesn't exist
        eventClass = env->FindClass("reflect/mobile/reflect/MainActivity$InputEvent");
        actionField = env->GetFieldID(eventClass, "action", "I");
        xField = env->GetFieldID(eventClass, "x", "F");
        yField = env->GetFieldID(eventClass, "y", "F");
        timestampField = env->GetFieldID(eventClass, "timestamp", "J");
        ANativeWindow* nativeWindow = ANativeWindow_fromSurface(env, surface);
        app = std::make_unique<rfct::reflectApplication>(nativeWindow);
    }

}




struct InputEvent {
    int action;
    float x, y;
    long timestamp;
};

extern "C"
JNIEXPORT void JNICALL
Java_reflect_mobile_reflect_MainActivity_sendEventsToNative(JNIEnv* env, jobject, jobject eventList) {
    jclass listClass = env->GetObjectClass(eventList);
    jmethodID sizeMethod = env->GetMethodID(listClass, "size", "()I");
    jmethodID getMethod = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
    jint size = env->CallIntMethod(eventList, sizeMethod);


    rfct::InputQueue::eventQueue.clear();
    for (int i = 0; i < size; i++) {
        jobject eventObj = env->CallObjectMethod(eventList, getMethod, i);
        int action = env->GetIntField(eventObj, actionField);
        float x = env->GetFloatField(eventObj, xField);
        float y = env->GetFloatField(eventObj, yField);
        long timestamp = env->GetLongField(eventObj, timestampField);

        rfct::InputQueue::eventQueue.push_back({action, x, y, timestamp});
        env->DeleteLocalRef(eventObj);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_reflect_mobile_reflect_MainActivity_renderNative(JNIEnv*, jobject) {
    for (const auto& event : rfct::InputQueue::eventQueue) {
        RFCT_TRACE("Event: action={}, x={}, y={}, timestamp={}", event.action, event.x, event.y,
             event.timestamp);
    }
    app->render();
    rfct::InputQueue::eventQueue.clear();
}

extern "C"
JNIEXPORT void JNICALL
Java_reflect_mobile_reflect_MainActivity_resizedSurface(JNIEnv *env, jobject thiz, int width, int height, jobject surface) {
    resizeCallback(width, height);
    if(width == 0 || height == 0)return;
    ANativeWindow* nativeWindow = ANativeWindow_fromSurface(env, surface);
    app->updateWindow(nativeWindow);
}


extern "C"
JNIEXPORT void JNICALL
Java_reflect_mobile_reflect_MainActivity_readAndCopyFile(JNIEnv *env, jobject thiz, jstring dirPath) {
    const char *cDirPath = env->GetStringUTFChars(dirPath, nullptr);
    rfct::reflectApplication::AssetsDirectory = std::string(cDirPath);
    env->ReleaseStringUTFChars(dirPath, cDirPath);
}