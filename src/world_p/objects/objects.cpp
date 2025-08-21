#include "objects.h"
#include "assets/scene_serialize_data.h"
#include "world_p/scene.h"
#include "world_p/objects/cigarettes.h"
#include "glm/gtc/matrix_transform.hpp"
#include "world_p/transform.h"
#include "world_p/object_components.h"

rfct::objectsHolder::~objectsHolder()
{
	cleanupCigarettes();
}

void rfct::objectsHolder::init(sceneSerializedData* serializeData, scene* parentScene)
{
	vines.reserve(serializeData->vines.size());
	for (vineInfo& vi : serializeData->vines) {
		vines.push_back(vine(vi.start, vi.end, vi.numEdges, parentScene));
	}
	nearestVineEdgeToPlayerIndex = -1;

	initCigaretteVars(parentScene);

	for (NPCInfo npcInfo : serializeData->npcs) {
		dynamicBoxColliderComponent boc = { npcInfo.min, npcInfo.max };
 		entity npcEntity = parentScene->createDynamicRect(&boc);
		npcEntity.set<dynamicObjectTypeComponent>({ dynamicObjectType::NPC });
		npcEntity.set<interactionDistanceComponent>({ npcInfo.ineratcionRadius * npcInfo.ineratcionRadius });
		npcEntity.set<dialoguePathComponent>({ npcInfo.dialogueFile });
		npcEntity.set<positionComponent>({ (boc.min + boc.max) * 0.5f });
		npcs.push_back(npcEntity);
	}
}
void rfct::objectsHolder::update(frameContext* fc)
{
	RFCT_PROFILE_SCOPE("cigarette update");
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
}

void rfct::objectsHolder::draw(const frameContext* fc){

	for (vine& v : vines) {
		v.draw(fc);
	}
	updateCigarettesMatrixes(fc);
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
