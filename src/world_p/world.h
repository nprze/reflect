#pragma once
#include <string>
#include "context.h"
#include "scene.h"
#include "render_data.h"

namespace rfct {
	class scene;
	class world {
	private:
		static world currentWorld;
	public:
		static world& getWorld() { return currentWorld; }
		void initWorld(const std::string& path);
		void loadScene(const std::string& path);
		inline scene& getCurrentScene() { return *m_currentScene; };
		void cleanWorld();
		void worldFixedUpdate(frameContext& context, uint64_t timesToUpdate);
		void worldVisualUpdate(frameContext& context);
        void addScreenTransform(float degree);
		renderData& getRenderData() { return *m_RenderData; };
		bool switchingScenes = false;
		void switchScenes(frameContext& ctx);
	private:
		scene* m_currentScene;
		renderData* m_RenderData = nullptr;
    public:
        float screenViewTransformDegrees = 0;
	};
}