#include "world.h"
#include "scene.h"
#include "input.h"
#include "job_system_p\job_system.h"
#include "renderer_p\debug\debug_draw.h"

rfct::world rfct::world::currentWorld;

void rfct::world::loadScene(const std::string& path)
{
	m_currentScene = new scene(this);
	m_currentScene->loadScene(path);
}

void rfct::world::cleanWorld() 
{ 
	m_currentScene->unloadScene(); 
	delete m_currentScene; 
}

void rfct::world::onUpdate(frameContext& context)
{
	context.scene = m_currentScene;
	auto jobs = std::make_shared<rfct::jobTracker>();
	jobSystem::get().KickJob([&]() {
		RFCT_PROFILE_SCOPE("Debug Draw");
		debugLine* line = debugDraw::requestLines(1);
		line[0].vertices[0].pos = { -0.25f, -0.25f, 0.f };
		line[0].vertices[1].pos = { 0.25f, -0.25f, 0.f };

		line[0].vertices[0].color = { 1.0f, 1.0f, 1.0f };
		line[0].vertices[1].color = { 1.0f, 1.0f, 1.0f };

		debugDraw::drawText("FPS: " + std::to_string(int(1 / context.dt)), glm::vec2(0, 0), 0.2);
		}, *jobs);
	jobSystem::get().KickJob([&]() {
		RFCT_PROFILE_SCOPE("Scene update");
		m_currentScene->onUpdate(&context);
		}, *jobs);
	jobs->waitUntil(2);
	RFCT_TRACE("yeepie");
}


