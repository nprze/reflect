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
	initObjects();
	//loadScene("scenes/load-test.txt");
	loadScene("scenes/showcase.txt");
}

void rfct::world::initObjects()
{
	objectsHolder::get().init();
}
 
void rfct::world::loadScene(const std::string& path)
{
	RFCT_PROFILE_FUNCTION();
	m_RenderData = new sceneRenderData();
	m_currentScene = new scene(this);
	createQueries(m_currentScene->sceneEntity);
	m_currentScene->loadScene(path);
}


void rfct::world::cleanWorld() 
{
	RFCT_PROFILE_FUNCTION();
	m_currentScene->unloadScene(); 
	objectsHolder::get().cleanup();
	delete m_currentScene; 
	delete m_RenderData;
}

void rfct::world::onUpdate(frameContext& context)
{
	if (m_currentScene->isPlayerOutsideScene()) {
		auto& world = ecs::get();
		world.delete_with(flecs::ChildOf, m_currentScene->sceneEntity);

		registerComponents();
		m_currentScene->unloadScene();
		m_RenderData->clearAllData();
		delete m_currentScene;
		m_currentScene = new scene(this);
		context.scene = m_currentScene;
		m_currentScene->loadScene("scenes/load-test.txt");
		//objectsHolder::get().switchScene(m_currentScene);
	}
	auto jobs = std::make_shared<rfct::jobTracker>();
	jobSystem::get().KickJob([&]() {
		RFCT_PROFILE_SCOPE("Debug Draw");
        debugDraw::drawText("FPS: " + std::to_string(int(1 / context.dt)), glm::vec2(0, 0), 0.2);
		}, *jobs);
	jobSystem::get().KickJob([&]() {
		RFCT_PROFILE_SCOPE("Scene update");
        m_currentScene->onUpdate(&context);
		}, *jobs);
#ifdef ANDROID_BUILD
    jobSystem::get().KickJob([&]() {
        RFCT_PROFILE_SCOPE("android UI update");
		input::getInput().drawButtons();
    }, *jobs);
#endif
	jobs->waitAll();
}


void rfct::world::addScreenTransform(float degree) {
	screenViewTransformDegrees = degree;
}
