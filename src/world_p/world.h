#pragma once
#include "scene.h"
#include <string>
namespace rfct {
	class world {
	private:
		static world currentWorld;
	public:
		static world& getWorld() { return currentWorld; }
		void loadScene(const std::string& path);
		inline scene& getCurrentScene() { return *m_currentScene; };
		void cleanWorld() ;
	private:
		world() {};
		scene* m_currentScene;
	};
}