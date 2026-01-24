#include "world.h"
#include "scene.h"
#include "input.h"
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

void rfct::world::worldFixedUpdate(frameContext& context, uint64_t timesToUpdate)
{
	while (timesToUpdate-- > 0) {
		if (context.state == gameState::gameplay || context.state == gameState::stateDialogue) {
			m_currentScene->FixedUpdate(&context);
		}
	}

	if (m_currentScene->isPlayerOutsideScene()) {
		switchingScenes = true;
	}
}

void rfct::world::worldVisualUpdate(frameContext& context)
{
	m_currentScene->onUpdate(&context);
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
