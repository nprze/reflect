#include "world.h"
#include "scene.h"
#include "input.h"
#include "job_system_p/job_system.h"
#include "renderer_p/debug/debug_draw.h"
#include "world_p/objects/objects.h"
#include "ecs.h"

rfct::world rfct::world::currentWorld;

void rfct::world::initWorld(const std::string& path)
{
	objectSystems::get().init();
	m_RenderData = new sceneRenderData();
	loadScene("scenes/load-test.txt");
}
 
void rfct::world::loadScene(const std::string& path)
{
	RFCT_PROFILE_FUNCTION();
	m_currentScene = new scene(this);
	m_currentScene->loadScene(path);
}


void rfct::world::cleanWorld() 
{
	RFCT_PROFILE_FUNCTION();
	m_currentScene->unloadScene(); 
	objectSystems::get().cleanup();
	delete m_currentScene; 
	delete m_RenderData;
}

void rfct::world::onUpdate(frameContext& context)
{
	if (m_currentScene->isPlayerOutsideScene()) {
		m_currentScene->unloadScene();
		delete m_currentScene;
		ecs::get().clear();
		m_RenderData->clearAllData();

		m_currentScene = new scene(this);
		loadScene("scenes/showcase.txt");
		context.scene = m_currentScene;
		//objectSystems::get().switchScene(m_currentScene);
	}
	//auto jobs = std::make_shared<rfct::jobTracker>();
	//jobSystem::get().KickJob([&]() {
	//	RFCT_PROFILE_SCOPE("Debug Draw");
        debugDraw::drawText("FPS: " + std::to_string(int(1 / context.dt)), glm::vec2(0, 0), 0.2);
	//	}, *jobs);
	//jobSystem::get().KickJob([&]() {
	//	RFCT_PROFILE_SCOPE("Scene update");
        m_currentScene->onUpdate(&context);
	//	}, *jobs);
#ifdef ANDROID_BUILD
    jobSystem::get().KickJob([&]() {
        RFCT_PROFILE_SCOPE("android UI update");
		input::getInput().drawButtons();
    }, *jobs);
#endif
	//jobs->waitAll();
}


void rfct::world::addScreenTransform(float degree) {
	screenViewTransformDegrees = degree;
}
