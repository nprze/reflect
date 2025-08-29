#pragma once
#include <string>
#include "context.h"
#include "scene.h"
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
		void onUpdate(frameContext& context);
        void addScreenTransform(float degree);
	private:
		world() = default;
		~world() = default;
		scene* m_currentScene;
    public:
        float screenViewTransformDegrees = 0;
	};
}