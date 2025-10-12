#include "objects.h"
#include "assets/scene_serialize_data.h"
#include "world_p/scene.h"
#include "glm/gtc/matrix_transform.hpp"
#include "world_p/transform.h"
#include "world_p/object_components.h"
#include "boosters/jump_booster.h"

rfct::objectsHolder rfct::objectsHolder::instance;

rfct::objectsHolder::~objectsHolder()
{
}

void rfct::objectsHolder::init()
{
	m_jumpBoostSystem.initSystem();
	m_spikeSystem.initSystem();
	m_npcSystem.initSystem();
	m_vineSystem.initSystem();
	m_enemySystem.initSystem();
	m_cigSystem.initSystem();
}

void rfct::objectsHolder::cleanup()
{
	m_cigSystem.cleanupSystem();
	m_enemySystem.cleanupSystem();
	m_vineSystem.cleanupSystem();
	m_npcSystem.cleanupSystem();
	m_spikeSystem.cleanupSystem();
	m_jumpBoostSystem.cleanupSystem();
}


void rfct::objectsHolder::loadSceneData(sceneSerializedData* serializeData, scene* parentScene)
{
	m_cigSystem.spawnData(parentScene, serializeData);
	m_enemySystem.spawnData(parentScene, serializeData);
	m_vineSystem.spawnData(parentScene, serializeData);
	m_npcSystem.spawnData(parentScene, serializeData);
	m_spikeSystem.spawnData(parentScene, serializeData);
	m_jumpBoostSystem.spawnData(parentScene, serializeData);
}

void rfct::objectsHolder::update(frameContext* fc)
{
	RFCT_PROFILE_SCOPE("dynamic objects update");
	// todo: make sure all the systems handle multiple fixed updates in one frame
	m_cigSystem.updateSystem(fc);
	m_enemySystem.updateSystem(fc);
	m_vineSystem.updateSystem(fc);
	m_npcSystem.updateSystem(fc);
	m_spikeSystem.updateSystem(fc);
	m_jumpBoostSystem.updateSystem(fc);
}

void rfct::objectsHolder::updateVisuals(frameContext* fc){
	m_cigSystem.updateVisuals(fc);
	m_enemySystem.updateVisuals(fc);
 	m_vineSystem.updateVisuals(fc);
	m_npcSystem.updateVisuals(fc);
	m_spikeSystem.updateVisuals(fc);
	m_jumpBoostSystem.updateVisuals(fc);
}
void rfct::objectsHolder::customDrawObjects(vk::CommandBuffer& cmd, frameContext* ctx)
{
	m_enemySystem.drawFrameAnimSprites(cmd, ctx);
}
void rfct::objectsHolder::reset(frameContext* fc)
{
	m_cigSystem.resetLevel(fc);
	m_vineSystem.resetLevel(fc);
}
void rfct::objectsHolder::onPlayerDash(frameContext* fc, const entity entityPlayer, const bool facingRight)
{
	m_cigSystem.onDash(fc, entityPlayer, facingRight);
}

void rfct::objectsHolder::onStartHolding(playerState state, nearestObject& nearest)
{
	if (nearest.vineIndex >= 0)
		m_vineSystem.onStartHolding(nearest);
}

void rfct::objectsHolder::onEndHolding()
{
	m_vineSystem.onEndHolding();
}