#pragma once
#include "renderer_p/renderer.h"
#include "game_p\game.h"
#include <string>
#include "assets\assets_manager.h"
#include "scene_p\scene_data.h"
namespace rfct {
	class reflectApplication {
	public:
        reflectApplication(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR);
		~reflectApplication() {};
		void render();
        static std::string AssetsDirectory;
    private:
        AssetsManager m_AssetsManager;
        unique<renderer> m_Renderer;
		camera m_camera;
		scene m_Scene;
		game::game m_Game;
	};
}