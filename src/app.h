#pragma once
#include "renderer_p/renderer.h"
#include <string>
#include "assets\assets_manager.h"
#include "input.h"
namespace rfct {
	class reflectApplication {
	public:
        reflectApplication(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR);
		~reflectApplication();

        void updateWindow(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR);
		void update();
		void updateGameplay(frameContext& ContextArg);
        static std::string AssetsDirectory;
        static bool shouldRender;
    private:
		size_t currentFrame = -1;
        AssetsManager m_AssetsManager;
        renderer m_Renderer;
		input m_Input;
	};
}