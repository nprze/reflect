#pragma once
#include "glm\glm.hpp"
#include "camera.h"
namespace rfct {
	class scene {
		static scene* m_currentScene;
	public:
		camera& m_camera;
		scene(camera& camArg): m_camera(camArg){};
		~scene() {};
		inline static scene* getCurrentScene() { return m_currentScene; };
		inline static void setCurrentScene(scene* sc) { m_currentScene = sc; };
		inline static void onUpdate() { m_currentScene->Update(); };
		void Update();

	};
}