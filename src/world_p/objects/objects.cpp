#include "objects.h"
#include "assets/scene_serialize_data.h"
#include "world_p/scene.h"
#include "world_p/objects/cigarettes.h"
#include "glm/gtc/matrix_transform.hpp"
#include "world_p/transform.h"
#include "world_p/object_components.h"
#include "world_p/objects/spikes.h"
#include "enemies/enemy.h"
#include "boosters/jump_booster.h"

rfct::objectsHolder::~objectsHolder()
{
	animBuffer.cleanup();
	cleanupCigarettes();
	cleanupSpikes();
	cleanupEnemies();
	cleanupJumpBoosterVars();
}

void rfct::objectsHolder::init(sceneSerializedData* serializeData, scene* parentScene)
{
	animBuffer.init(10000);
	initCigaretteVars(parentScene);
	initSpikeVars(parentScene);
	spawnEnemies(serializeData, parentScene, &animBuffer);
	initJumpBoosterVars(parentScene, serializeData);


	vines.reserve(serializeData->vines.size());
	for (vineInfo& vi : serializeData->vines) {
		vines.push_back(vine(vi.start, vi.end, vi.numEdges, parentScene));
	}
	nearestVineEdgeToPlayerIndex = -1;


	for (NPCInfo npcInfo : serializeData->npcs) {
		dynamicBoxColliderComponent boc = { npcInfo.min, npcInfo.max };
 		entity npcEntity = parentScene->createDynamicRect(&boc);
		npcEntity.set<dynamicObjectTypeComponent>({ dynamicObjectType::NPC });
		npcEntity.set<interactionDistanceComponent>({ npcInfo.ineratcionRadius * npcInfo.ineratcionRadius });
		npcEntity.set<dialoguePathComponent>({ npcInfo.dialogueFile });
		npcEntity.set<positionComponent>({ (boc.min + boc.max) * 0.5f });
		npcs.push_back(npcEntity);
	}

	for (SpikeInfo spike : serializeData->spikes) {
		createSpike(parentScene, spike);
	}
}
void rfct::objectsHolder::update(frameContext* fc)
{
	RFCT_PROFILE_SCOPE("dynamic objects update");
	if (fc->fixedUpdateTimes) {
		if (nearestVineEdgeToPlayerIndex != -1) {
			if (vineClosestToPlayer.get<vineStateComponent>()->holdingToThis) {
				fc->scene->getPlayer().get_mut<positionComponent>()->position = simulateVinePlayerIsHolding(fc->scene->getPlayer(), vineClosestToPlayer, nearestVineEdgeToPlayerIndex, fc);
			}

		}
		for (entity& npcEntity : npcs) {
			updateNpc(fc, npcEntity, this);
		}
		for (vine& v : vines) {
			v.update(fc);
		}
		updateCigarettes(fc);
	}
	updateEnemies(fc);
	updateJumpBoosters(fc);
}

void rfct::objectsHolder::updateMatrices(frameContext* fc){

	for (vine& v : vines) {
		v.draw(fc);
	}
	updateCigarettesMatrixes(fc);
	updateEnemiesMatrices(fc);
}
void rfct::objectsHolder::customDrawObjects(vk::CommandBuffer& cmd, frameContext* ctx)
{
	drawEnemies(cmd, ctx);
}
void rfct::objectsHolder::reset(frameContext* fc)
{
	for (vine& v : vines) {
		v.reset();
	}

	resetCigarettes(fc);
	cigarettes.clear();
	cigarettes.resize(cigarettesMaxCount);
}
void rfct::objectsHolder::onPlayerDashObjects(frameContext* fc, const entity entityPlayer, const bool facingRight)
{
	entity newCigarette = constructCigarette(fc, entityPlayer, facingRight);
	if (cigarettes[lastCigaretteIndex] != entity()) {
		fc->scene->deleteDynamicEntity(cigarettes[lastCigaretteIndex]);
	}
	cigarettes[lastCigaretteIndex] = (newCigarette);
	lastCigaretteIndex = (lastCigaretteIndex + 1) % cigarettesMaxCount;
}
