#pragma once
#include <string>
#include "context.h"
#include "scene.h"
#include "render_data.h"

//only for debug
#include "sound_p/sound.h"
namespace rfct {
	class scene;
	class world {
		sound bg;
	private:
		static world currentWorld;
	public:
		static world& getWorld() { return currentWorld; }
		void initWorld(const std::string& path);
		void loadScene(const std::string& path);
		inline scene& getCurrentScene() { return *m_currentScene; };
		void cleanWorld();
		void onUpdate(frameContext& context);
        void addScreenTransform(float degree);
		sceneRenderData& getRenderData() {
			return *m_RenderData;
		};
	private:
		world() = default;
		~world() = default;
		scene* m_currentScene;
		sceneRenderData* m_RenderData = nullptr;
    public:
        float screenViewTransformDegrees = 0;
	};
}