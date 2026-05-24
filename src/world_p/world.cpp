#include "world.h"
#include "scene.h"
#include "input.h"
#include "renderer_p/debug/debug_draw.h"
#include "world_p/objects/objects.h"
#include "ecs.h"
#include "ui_p/ui.h"
#include "assets/object_load.h"
#include "render_data.h"

rfct::world currentWorld;

rfct::world& rfct::world::getWorld() {
	return currentWorld;
}

void rfct::world::initWorld(const std::string& path) {
	RFCT_PROFILE_FUNCTION();
	loadWorld(path, &m_serializeData);
	m_RenderData = new renderData();
	loadScene("scenes/"+m_serializeData.blocks[m_currentWorldBlockIndex].file + ".txt");
}
 
void rfct::world::loadScene(const std::string& path) {
	RFCT_PROFILE_FUNCTION();
	m_currentScene = new scene(this);
	m_currentScene->initScene(path);
}


void rfct::world::cleanWorld() {
	RFCT_PROFILE_FUNCTION();
	delete m_currentScene; 
	delete m_RenderData;
}

void rfct::world::worldFixedUpdate(frameContext& context, uint64_t timesToUpdate) {
	RFCT_PROFILE_FUNCTION();
	while (timesToUpdate-- > 0) {
		if (context.state == gameState::gameplay || context.state == gameState::stateDialogue) {
			m_currentScene->FixedUpdate(&context);
		}
	}
	m_currentScene->postFixedUpdate(&context);

	if (m_currentScene->isPlayerOutsideScene()) {
		switchingScenes = true;
	}
}

void rfct::world::startSwitchScene(frameContext& ctx) {
	RFCT_INFO("pending switch scene");
}

void rfct::world::worldVisualUpdate(frameContext& context) {
	if (context.state == gameState::menu) return;
	m_currentScene->onUpdate(&context);
}

void rfct::world::addScreenTransform(float degree) {
	screenViewTransformDegrees = degree;
}

void rfct::world::switchScenes(frameContext& ctx) {
	RFCT_PROFILE_FUNCTION();
	glm::vec2 coords = m_currentScene->getPlayerCoordsSceneNormalized();
	coords.y = 1 - coords.y;
	RFCT_INFO("switching scenes using pos: {}, {}", coords.x, coords.y);
	uint32_t sceneIndex = getSceneToLoad(coords);
	if (m_currentWorldBlockIndex == sceneIndex) {
		m_currentScene->resetScene(&ctx);
		return;
	}
	m_currentWorldBlockIndex = sceneIndex;
	RFCT_INFO("new scene name: {}", m_serializeData.blocks[m_currentWorldBlockIndex].file);

	delete m_currentScene;
	ecs::get().clear();
	m_RenderData->clearAllData();
	loadScene("scenes/"+ m_serializeData.blocks[m_currentWorldBlockIndex].file +".txt");
	ctx.scene = m_currentScene;
}

uint32_t rfct::world::getSceneToLoad(glm::vec2& lastBlockExit) {
	uint32_t currentIndex = 0;
	glm::vec2 levelSize = m_serializeData.blocks[m_currentWorldBlockIndex].max - m_serializeData.blocks[m_currentWorldBlockIndex].min;
	glm::vec2 pos = m_serializeData.blocks[m_currentWorldBlockIndex].min + (lastBlockExit * levelSize);
	if (lastBlockExit.x == 1) {
		pos.x += 1;
	}
	if (lastBlockExit.x == 0) {
		pos.x -= 1;
	}
	if (lastBlockExit.y == 1) {
		pos.y += 1;
	}
	if (lastBlockExit.y == 0) {
		pos.y -= 1;
	}
	for (blockSerializeData& b : m_serializeData.blocks) {
		if (pos.x > b.min.x &&
			pos.x < b.max.x &&
			pos.y > b.min.y &&
			pos.y < b.max.y) {
			return currentIndex;
		}
		currentIndex++;
	}
	return m_currentWorldBlockIndex;
}
