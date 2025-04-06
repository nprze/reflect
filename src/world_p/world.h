#pragma once
#include "scene.h"
#include <string>
#include "context.h"
#include "job_system_p\job_system.h"
namespace rfct {
	class world {
	private:
		static world currentWorld;
	public:
		static world& getWorld() { return currentWorld; }
		void loadScene(const std::string& path);
		inline scene& getCurrentScene() { return *m_currentScene; };
		void cleanWorld();
		void onUpdate(frameContext& context);
		jobSystem& getJobSystem() { return m_jobSystem; }
	private:
		jobSystem m_jobSystem;
		world();
		~world();
		scene* m_currentScene;
	};
}