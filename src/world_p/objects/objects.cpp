#include "objects.h"
#include "assets/scene_serialize_data.h"
#include "world_p/scene.h"
#include "world_p/objects/cigarettes.h"
#include "glm/gtc/matrix_transform.hpp"
#include "world_p/transform.h"

void rfct::objectsHolder::init(sceneSerializedData* serializeData, scene* parentScene)
{
	vines.reserve(serializeData->vines.size());
	for (vineInfo& vi : serializeData->vines) {
		vines.push_back(vine(vi.start, vi.end, vi.numEdges, parentScene));
	}
	nearestVineEdgeToPlayerIndex = -1;


	constexpr float between = 0.10f;
	constexpr float rest = .15f;

	glm::vec3 blue = { 0.f,0.f, 1.f };
	glm::vec3 black = { 0.f,0.f, 0.f };
	// background triangles
	glm::vec3 bg0 = glm::vec3(-rest, rest, 0);
	glm::vec3 bg1 = glm::vec3(rest, rest, 0);
	glm::vec3 bg2 = glm::vec3(-rest, -rest, 0);
	glm::vec3 bg3 = glm::vec3(rest, -rest, 0);


	cigaretteVertices[0].pos = bg0;
	cigaretteVertices[1].pos = bg1;
	cigaretteVertices[2].pos = bg2;

	cigaretteVertices[3].pos = bg3;
	cigaretteVertices[4].pos = bg1;
	cigaretteVertices[5].pos = bg2;

	for (uint8_t i = 0; i < 6; ++i) {
		cigaretteVertices[i].color = black;
	}


	// color triangles
	glm::vec3 v0 = glm::vec3(-between, between, 0);
	glm::vec3 v1 = glm::vec3(between, between, 0);
	glm::vec3 v2 = glm::vec3(-between, -between, 0);
	glm::vec3 v3 = glm::vec3(between, -between, 0);


	cigaretteVertices[6].pos = v2;
	cigaretteVertices[7].pos = v1;
	cigaretteVertices[8].pos = v3;

	cigaretteVertices[9].pos = v0;
	cigaretteVertices[10].pos = v1;
	cigaretteVertices[11].pos = v2;


	for (uint8_t i = 6; i < 12; ++i) {
		cigaretteVertices[i].color = blue;
	}
}
void rfct::objectsHolder::update(const frameContext* fc)
{
	if (fc->fixedUpdateTimes) {
		if (nearestVineEdgeToPlayerIndex != -1) {
			if (vineClosestToPlayer.get<vineStateComponent>()->holdingToThis) {
				fc->scene->getPlayer().get_mut<positionComponent>()->position = simulateVinePlayerIsHolding(fc->scene->getPlayer(), vineClosestToPlayer, nearestVineEdgeToPlayerIndex, fc);
			}

		}
		for (vine& v : vines) {
			v.update(fc);
		}
		for (entity& cigarette : cigarettes) {
			if (cigarette == entity()) return;
			updateCigarette(cigarette, fc);
		}
	}
	for (vine& v : vines) {
		v.draw(fc);
	}
}

void rfct::objectsHolder::onPlayerDash(const frameContext* fc, const entity entityPlayer, const bool facingRight)
{

	glm::mat4 transMat = glm::translate(glm::mat4(1.f), glm::vec3(entityPlayer.get<positionComponent>()->position, 0.f));
 	entity newCigarette = fc->scene->createDynamicRenderingEntity(&cigaretteVertices, &transMat);
	newCigarette.set<positionComponent>({ entityPlayer.get<positionComponent>()->position });
	newCigarette.set<staticObjCollisionCallbackComponent>({ onCollision_Cigarette_StaticObj });
	newCigarette.set<rotationComponent>({});
	newCigarette.set<scaleComponent>({});
	newCigarette.set<gravityComponent>({ 0.97f, false, 5.f});
	newCigarette.set<velocityComponent>({ .5f * glm::vec2{facingRight ? -1.f : 1.f, 1.f} });
	newCigarette.set<dynamicObjectTypeComponent>({ dynamicObjectType::Cigarette });
	newCigarette.set<dynamicBoxColliderComponent>({});
	constructCigaretteBoundingBox(newCigarette);
	if (cigarettes[lastCigaretteIndex]!=entity())
		cigarettes[lastCigaretteIndex].destruct();
	cigarettes[lastCigaretteIndex] = (newCigarette);
	lastCigaretteIndex = (lastCigaretteIndex + 1) % cigarettesMaxCount;
}

void rfct::objectsHolder::constructCigaretteBoundingBox(entity cigarette)
{
	dynamicBoxColliderComponent* boc = cigarette.get_mut<dynamicBoxColliderComponent>();
	const positionComponent* pos = cigarette.get<positionComponent>();
	glm::mat4 tranformations = getModelMatrixFromEntity(cigarette);
	boc->max = { FLT_MIN, FLT_MIN };
	boc->min = { FLT_MAX, FLT_MAX };
	for (uint8_t i = 0; i < 4; ++i) {
		boc->min.x = std::min(cigaretteVertices[i].pos.x, boc->min.x);
		boc->min.y = std::min(cigaretteVertices[i].pos.y, boc->min.y);

		boc->max.x = std::max(cigaretteVertices[i].pos.x, boc->max.x);
		boc->max.y = std::max(cigaretteVertices[i].pos.y, boc->max.y);
	}
}

