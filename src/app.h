#pragma once
#include "renderer_p/renderer.h"
namespace rfct {
	class reflectApplication {
	public:
        reflectApplication(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR);
		~reflectApplication() {};
		void render();
    private:
        unique<renderer> m_Renderer;
	};
}