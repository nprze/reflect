#pragma once
#include "vine.h"
#include "context.h"

namespace rfct{
	struct sceneSerializedData;
	struct objectsHolder {
		void init(sceneSerializedData* serializeData, scene* parentScene);
		void update(const frameContext* fc);
		std::vector<vine> vines;
		entity vineClosestToPlayer;
		int nearestVineEdgeToPlayerIndex;
		glm::vec2 nearestVineEdgeToPlayerPosition;


	};
}