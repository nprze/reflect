#pragma once
#include "renderer_p/renderer.h"
#include "input.h"
namespace rfct {
	class reflectApplication {
	public:
        reflectApplication(RFCT_APP_ARGS); // arguments vary by platform
		~reflectApplication();

        void updateWindow(RFCT_APP_ARGS);
		void update();
		void updateGameplay(frameContext& ContextArg);

        static bool isAppMinimised;
    private:
		size_t currentFrame = -1; // the frame in flight which resources to use this frame
        renderer m_Renderer;
	};
}