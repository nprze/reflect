#include "objects.h"
#include "world_p/components.h"
#include "world_p/physics/physics.h"
#include "world_p/ecs.h"

rfct::objectSystems rfct::objectSystems::instance;


void rfct::objectSystems::init()
{
	m_jumpBoostSystem.initSystem();
	m_spikeSystem.initSystem();
	m_npcSystem.initSystem();
	m_vineSystem.initSystem();
	m_enemySystem.initSystem();
	m_cigSystem.initSystem();

	createAllQueries();
}

void rfct::objectSystems::cleanup()
{
	m_cigSystem.cleanupSystem();
	m_enemySystem.cleanupSystem();
	m_vineSystem.cleanupSystem();
	m_npcSystem.cleanupSystem();
	m_spikeSystem.cleanupSystem();
	m_jumpBoostSystem.cleanupSystem();

	cleanupAllQueries();
}

void rfct::objectSystems::createAllQueries()
{
	m_jumpBoostSystem.createQueries();
	m_spikeSystem.createQueries();
	m_npcSystem.createQueries();
	m_vineSystem.createQueries();
	m_enemySystem.createQueries();
	m_cigSystem.createQueries();
}

void rfct::objectSystems::cleanupAllQueries()
{
	m_jumpBoostSystem.deleteQueries();
	m_spikeSystem.deleteQueries();
	m_npcSystem.deleteQueries();
	m_vineSystem.deleteQueries();
	m_enemySystem.deleteQueries();
	m_cigSystem.deleteQueries();
}


void rfct::objectSystems::loadSceneData(sceneSerializedData* serializeData, scene* parentScene)
{
	m_cigSystem.spawnData(parentScene, serializeData);
	m_enemySystem.spawnData(parentScene, serializeData);
	m_vineSystem.spawnData(parentScene, serializeData);
	m_npcSystem.spawnData(parentScene, serializeData);
	m_spikeSystem.spawnData(parentScene, serializeData);
	m_jumpBoostSystem.spawnData(parentScene, serializeData);
}

void rfct::objectSystems::update(frameContext* fc)
{
	RFCT_PROFILE_SCOPE("dynamic objects update");
	// todo: make sure all the systems handle delay (multiple fixed updates in one frame)
	m_cigSystem.updateSystem(fc);
	m_enemySystem.updateSystem(fc);
	m_vineSystem.updateSystem(fc);
	m_npcSystem.updateSystem(fc);
	m_spikeSystem.updateSystem(fc);
	m_jumpBoostSystem.updateSystem(fc);
}

void rfct::objectSystems::updateVisuals(frameContext* fc){
	m_cigSystem.updateVisuals(fc);
	m_enemySystem.updateVisuals(fc);
 	m_vineSystem.updateVisuals(fc);
	m_npcSystem.updateVisuals(fc);
	m_spikeSystem.updateVisuals(fc);
	m_jumpBoostSystem.updateVisuals(fc);
}
void rfct::objectSystems::customDrawObjects(vk::CommandBuffer& cmd, frameContext* ctx)
{
	m_enemySystem.drawFrameAnimSprites(cmd, ctx);
}
void rfct::objectSystems::respawn(frameContext* fc)
{
	m_cigSystem.resetLevel(fc);
	m_vineSystem.resetLevel(fc);
}
void rfct::objectSystems::onPlayerDash(frameContext* fc, const entity entityPlayer, const bool facingRight)
{
	m_cigSystem.onDash(fc, entityPlayer, facingRight);
}

void rfct::objectSystems::onStartHolding(playerState state, nearestObject& nearest)
{
	if (nearest.vineIndex >= 0)
		m_vineSystem.onStartHolding(nearest);
}

void rfct::objectSystems::onEndHolding()
{
	m_vineSystem.onEndHolding();
}

void rfct::objectSystems::onSwitchScene()
{
	cleanupAllQueries();
	
	ecs::recreateWorld();

	createAllQueries();
}
