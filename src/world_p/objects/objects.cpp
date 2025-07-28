#include "objects.h"
#include "assets/scene_serialize_data.h"
#include "world_p/scene.h"

void rfct::objectsHolder::init(sceneSerializedData* serializeData, scene* parentScene)
{
	vines.reserve(serializeData->vines.size());
	for (vineInfo& vi : serializeData->vines) {
		vines.push_back(vine(vi.start, vi.end, vi.numEdges, parentScene));
	}
	nearestVineEdgeToPlayerIndex = -1;
}
void rfct::objectsHolder::update(const frameContext* fc)
{
	if (nearestVineEdgeToPlayerIndex != -1) {
		if (vineClosestToPlayer.get<vineStateComponent>()->holdingToThis) {
			fc->scene->getPlayer().get_mut<positionComponent>()->position = simulateVinePlayerIsHolding(fc->scene->getPlayer(), vineClosestToPlayer, nearestVineEdgeToPlayerIndex, fc);
		}

	}
	for (vine& v : vines) {
		v.draw();
	}
	if (fc->fixedUpdateTimes) {
		for (vine& v : vines) {
			v.update(fc);
		}
	}
}

