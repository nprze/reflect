#pragma once
#include "renderer_p/renderer.h"
namespace rfct {
	class reflectApplication {
	public:
		reflectApplication(ANativeWindow* window);
		~reflectApplication() {};
		void render();
    private:
        unique<renderer> m_Renderer;
	};
}