#pragma once
#include <string>
#include "context.h"
namespace rfct {
	class scene;
	class world {
	private:
		static world currentWorld;
	public:
		static world& getWorld() { return currentWorld; }
		void loadScene(const std::string& path);
		inline scene& getCurrentScene() { return *m_currentScene; };
		void cleanWorld();
		void onUpdate(frameContext& context);
	private:
		world() = default;
		~world() = default;
		scene* m_currentScene;
	};
}