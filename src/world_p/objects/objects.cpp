#include "objects.h"
#include "world_p/components.h"
#include "world_p/physics/physics.h"

rfct::objectSystems instance;
rfct::objectSystems& rfct::objectSystems::get() { return instance; };

void rfct::objectSystems::init() {
	RFCT_PROFILE_FUNCTION();
	m_jumpBoostSystem.initSystem();
	m_spikeSystem.initSystem();
	m_npcSystem.initSystem();
	m_vineSystem.initSystem();
	m_enemySystem.initSystem();
	m_cigSystem.initSystem();
}

void rfct::objectSystems::cleanupBuffer() {
	RFCT_PROFILE_FUNCTION();
	m_cigSystem.cleanupSystem();
	m_enemySystem.cleanupSystem();
	m_vineSystem.cleanupSystem();
	m_npcSystem.cleanupSystem();
	m_spikeSystem.cleanupSystem();
	m_jumpBoostSystem.cleanupSystem();
}

void rfct::objectSystems::loadSceneData(sceneSerializedData* serializeData, scene* parentScene) {
	RFCT_PROFILE_FUNCTION();
	m_cigSystem.spawnData(parentScene, serializeData);
	m_enemySystem.spawnData(parentScene, serializeData);
	m_vineSystem.spawnData(parentScene, serializeData);
	m_npcSystem.spawnData(parentScene, serializeData);
	m_spikeSystem.spawnData(parentScene, serializeData);
	m_jumpBoostSystem.spawnData(parentScene, serializeData);
}

void rfct::objectSystems::systemsFixedUpdate(frameContext* fc) {
	RFCT_PROFILE_FUNCTION();
	m_cigSystem.updateSystem(fc);
	m_enemySystem.updateSystem(fc);
	m_vineSystem.updateSystem(fc);
	m_npcSystem.updateSystem(fc);
	m_spikeSystem.updateSystem(fc);
	m_jumpBoostSystem.updateSystem(fc);
}

void rfct::objectSystems::updateVisuals(frameContext* fc){
	RFCT_PROFILE_FUNCTION();
	m_cigSystem.updateVisuals(fc);
	m_enemySystem.updateVisuals(fc);
 	m_vineSystem.updateVisuals(fc);
	m_npcSystem.updateVisuals(fc);
	m_spikeSystem.updateVisuals(fc);
	m_jumpBoostSystem.updateVisuals(fc);
}
void rfct::objectSystems::customDrawObjects(vk::CommandBuffer& cmd, frameContext* ctx) {
	RFCT_PROFILE_FUNCTION();
	m_enemySystem.drawFrameAnimSprites(cmd, ctx);
	// TODO: grass should be drawn here to be displayed over
}
void rfct::objectSystems::respawn(frameContext* fc) {
	RFCT_PROFILE_FUNCTION();
	m_cigSystem.resetLevel(fc);
	m_vineSystem.resetLevel(fc);
}
void rfct::objectSystems::onPlayerDash(frameContext* fc, const entity entityPlayer, const bool facingRight) {
	RFCT_PROFILE_FUNCTION();
	m_cigSystem.onDash(fc, entityPlayer, facingRight);
}

void rfct::objectSystems::onStartHolding(playerState state, nearestObject& nearest) {
	RFCT_PROFILE_FUNCTION();
	if (nearest.vineIndex >= 0)
		m_vineSystem.onStartHolding(nearest);
}

void rfct::objectSystems::onEndHolding() {
	RFCT_PROFILE_FUNCTION();
	m_vineSystem.onEndHolding();
}
