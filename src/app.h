#pragma once
#include "renderer_p/renderer.h"
#include "game_p\game.h"
#include <string>
#include "assets\assets_manager.h"
#include "input.h"
#include "world_p\world.h"
namespace rfct {
	class reflectApplication {
	public:
        reflectApplication(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR);
		~reflectApplication();

        void updateWindow(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR);
		void render();
        static std::string AssetsDirectory;
        static bool shouldRender;
    private:
        AssetsManager m_AssetsManager;
        unique<renderer> m_Renderer;
		input m_Input;
		game::game m_Game;
	};
}