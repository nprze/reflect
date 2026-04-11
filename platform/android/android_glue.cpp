#include <jni.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include "app.h"
#include <iostream>
#include "renderer_p/renderer.h"
#include <vector>
#include <android/log.h>
#include "android_glue.h"
#include "assets/assets_manager.h"

// BEGIN_INCLUDE(all)
#include <android_native_app_glue.h>
#include <cstdlib>
#include <cstring>

#include <android/native_activity.h>
#include <android/input.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <sys/stat.h>
#include <unistd.h>

void copyAssetFile(AAssetManager* mgr, const char* assetPath, const char* targetPath) {
    AAsset* asset = AAssetManager_open(mgr, assetPath, AASSET_MODE_STREAMING);
    if (!asset) {
        RFCT_CRITICAL("Failed to open asset: {}", assetPath);
        return;
    }

    FILE* out = fopen(targetPath, "wb");
    if (!out) {
        RFCT_CRITICAL("Failed to open output: {}", targetPath);
        AAsset_close(asset);
        return;
    }

    char buf[4096];
    int read;
    while ((read = AAsset_read(asset, buf, sizeof(buf))) > 0) {
        fwrite(buf, 1, read, out);
    }

    fclose(out);
    AAsset_close(asset);
}

// Reads the directories.txt file from assets and returns a list of directories to copy
std::vector<std::string> readAssetDirectories(AAssetManager* mgr, const char* assetDir) {
    std::vector<std::string> dirs;
    std::string path = std::string(assetDir) + "directories.txt";
    AAsset* asset = AAssetManager_open(mgr, path.c_str(), AASSET_MODE_BUFFER);
    if (!asset) {
        RFCT_CRITICAL("Failed to open directories.txt at {}", path);
        return dirs;
    }

    size_t size = AAsset_getLength(asset);
    std::string content(size, '\0');
    AAsset_read(asset, &content[0], size);
    AAsset_close(asset);

    std::istringstream ss(content);
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (!line.empty()) dirs.push_back(line);
    }

    return dirs;
}

void copyAssetDirFiltered(AAssetManager* mgr, const char* targetDir) {
    // Get allowed subdirectories
    std::vector<std::string> allowedDirs = readAssetDirectories(mgr, "");
    if (allowedDirs.empty()) return;

    mkdir(targetDir, 0755);

    for (const std::string& subDirName : allowedDirs) {
        std::string assetPath = subDirName;
        std::string targetPath = std::string(targetDir) + "/" + subDirName;

        AAssetDir* dir = AAssetManager_openDir(mgr, assetPath.c_str());
        if (!dir) continue;

        mkdir(targetPath.c_str(), 0755);

        const char* filename;
        while ((filename = AAssetDir_getNextFileName(dir)) != nullptr) {
            std::string fileAssetPath = assetPath + "/" + filename;
            std::string fileTargetPath = targetPath + "/" + filename;

            copyAssetFile(mgr, fileAssetPath.c_str(), fileTargetPath.c_str());
        }

        AAssetDir_close(dir);
    }
}


std::vector<rfct::InputEvent> rfct::InputQueue::eventQueue;

static std::unique_ptr<rfct::reflectApplication> app;

struct SavedState {
    int32_t x;
    int32_t y;
};

struct Engine {
    android_app* app{};

    ANativeWindow* window = nullptr;
    int32_t width = 0;
    int32_t height = 0;
    SavedState state{};

private:
    bool running_ = false;
};

static void engine_handle_cmd(android_app* app, int32_t cmd) {
    Engine* engine = reinterpret_cast<Engine*>(app->userData);
    switch (cmd) {
    case APP_CMD_SAVE_STATE:
        app->savedState = malloc(sizeof(SavedState));
        *(SavedState*)app->savedState = engine->state;
        app->savedStateSize = sizeof(SavedState);
        break;
    case APP_CMD_INIT_WINDOW:
        if (app->window != nullptr) {
            engine->window = app->window;
            engine->width = ANativeWindow_getWidth(engine->window);
            engine->height = ANativeWindow_getHeight(engine->window);
            RFCT_INFO("Window initialized: %dx%d", engine->width, engine->height);
        }
        break;
    case APP_CMD_TERM_WINDOW:
        engine->window = nullptr;
        break;
    case APP_CMD_GAINED_FOCUS:
        break;
    case APP_CMD_LOST_FOCUS:
        break;
    default:
        break;
    }
}


void android_main(android_app* state) {
    Engine engine{};
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = [](android_app* app, AInputEvent* event) -> int32_t {
        if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
            int pointerCount = AMotionEvent_getPointerCount(event);

            for (int i = 0; i < pointerCount; ++i) {
                rfct::InputEvent inputEvent;

                // Get the pointer ID
                inputEvent.pointerID = AMotionEvent_getPointerId(event, i);

                // Get coordinates
                inputEvent.x = AMotionEvent_getX(event, i);
                inputEvent.y = AMotionEvent_getY(event, i);

                // Get the action for this pointer
                int actionMasked = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;

                inputEvent.action = actionMasked;

                // Add to the input queue
                rfct::InputQueue::eventQueue.push_back(inputEvent);
            }
            //RFCT_INFO("motion event");
            return 1; // Event handled
        }
        return 0; // Not a motion event
        };

    AAssetManager* mgr = state->activity->assetManager;
    std::string outDir = state->activity->internalDataPath; // /data/data/<pkg>/files

    copyAssetDirFiltered(mgr, outDir.c_str());

    engine.app = state;

    if (state->savedState != nullptr) {
        engine.state = *(SavedState*)state->savedState;
    }
    rfct::InputQueue::eventQueue.reserve(5);
    while (!state->destroyRequested) {
        android_poll_source* source = nullptr;
        int result = ALooper_pollOnce(16, nullptr, nullptr, reinterpret_cast<void**>(&source));
        if (result == ALOOPER_POLL_ERROR) {
            RFCT_ERROR("ALooper_pollOnce returned an error");
            break;
        }
        if (source) source->process(state, source);
        if (!app && state->window) {
            rfct::AssetsManager::get().init(std::string(outDir.c_str()));
            app = std::make_unique<rfct::reflectApplication>(state->window);
            break;
        }
    }
    while (!state->destroyRequested) {
        android_poll_source* source = nullptr;
        int result = ALooper_pollOnce(16, nullptr, nullptr, reinterpret_cast<void**>(&source));
        if (result == ALOOPER_POLL_ERROR) {
            RFCT_ERROR("ALooper_pollOnce returned an error");
            break;
        }
        if (source) source->process(state, source);
        app->update();
        rfct::InputQueue::eventQueue.clear();
    }
}
// END_INCLUDE(all)

/*
void resizeCallback(int width, int height){
    rfct::renderer::getRen().getRenderImagesManager().getSwapChain().framebufferResized = true;
    if (width == 0 && height == 0)
    {
        rfct::reflectApplication::isAppMinimised = true;
    }
    else {
        rfct::reflectApplication::isAppMinimised = false;
    }

}
struct InputEvent {
    int action;
    float x, y;
    int pointerID;
};

std::vector<rfct::InputEvent> rfct::InputQueue::eventQueue;

static std::unique_ptr<rfct::reflectApplication> app;
static jclass eventClass;
static jfieldID actionField;
static jfieldID xField;
static jfieldID yField;
static jfieldID pointerIDField;
extern "C" JNIEXPORT void JNICALL
Java_reflect_mobile_reflect_MainActivity_createVulkanApp(JNIEnv *env, jobject thiz, jobject surface) {
    if (!app) {  // the code is also called when the app is unminimized
        eventClass = env->FindClass("reflect/mobile/reflect/MainActivity$InputEvent");
        actionField = env->GetFieldID(eventClass, "action", "I");
        xField = env->GetFieldID(eventClass, "x", "F");
        yField = env->GetFieldID(eventClass, "y", "F");
        pointerIDField = env->GetFieldID(eventClass, "pointerID", "I");
        ANativeWindow* nativeWindow = ANativeWindow_fromSurface(env, surface);
        app = std::make_unique<rfct::reflectApplication>(nativeWindow);
    }

}





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
        int pointerID = env->GetIntField(eventObj, pointerIDField);
        rfct::InputQueue::eventQueue.push_back({action, x, y, pointerID});
        env->DeleteLocalRef(eventObj);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_reflect_mobile_reflect_MainActivity_renderNative(JNIEnv*, jobject) {

    app->update();
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
    rfct::AssetsManager::get().init(std::string(cDirPath));
    env->ReleaseStringUTFChars(dirPath, cDirPath);
}*/