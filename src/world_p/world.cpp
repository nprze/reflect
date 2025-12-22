#include "world.h"
#include "scene.h"
#include "input.h"
#include "job_system_p/job_system.h"
#include "renderer_p/debug/debug_draw.h"
#include "world_p/objects/objects.h"
#include "ecs.h"
#include "ui_p/ui.h"

rfct::world rfct::world::currentWorld;

void rfct::world::initWorld(const std::string& path)
{
	m_RenderData = new renderData();
	loadScene("scenes/cool-scene.txt");
}
 
void rfct::world::loadScene(const std::string& path)
{
	RFCT_PROFILE_FUNCTION();
	m_currentScene = new scene(this);
	m_currentScene->initScene(path);
}


void rfct::world::cleanWorld() 
{
	RFCT_PROFILE_FUNCTION();
	m_currentScene->unloadScene(); 
	delete m_currentScene; 
	delete m_RenderData;
}

void rfct::world::onUpdate(frameContext& context)
{
	auto jobs = std::make_shared<rfct::jobTracker>();
	if (context.state == gameState::gameplay || context.state == gameState::stateDialogue) {
		jobSystem::get().KickJob([&]() {
			RFCT_PROFILE_SCOPE("Scene update");
				m_currentScene->onUpdate(&context);
			}, *jobs);
	}
	jobs->waitAll();

	if (m_currentScene->isPlayerOutsideScene()) {
		switchingScenes = true;
	}
}


void rfct::world::addScreenTransform(float degree) {
	screenViewTransformDegrees = degree;
}

void rfct::world::switchScenes(frameContext& ctx)
{
	m_currentScene->unloadScene();
	delete m_currentScene;
	ecs::get().clear();
	m_RenderData->clearAllData();
	loadScene("scenes/showcase.txt");
	ctx.scene = m_currentScene;
}
