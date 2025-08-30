#pragma once
#include "vine.h"
#include "npc.h"
#include "context.h"

namespace rfct{
	struct sceneSerializedData;
	constexpr uint8_t cigarettesMaxCount = 4;
	struct objectsHolder {
		objectsHolder() : cigarettes(cigarettesMaxCount, entity()) {};
		~objectsHolder();
		void init(sceneSerializedData* serializeData, scene* parentScene);
		void update(frameContext* fc);
		void draw(const frameContext* fc);
		void reset(const frameContext* fc);
		
		void onPlayerDashObjects(frameContext* fc, const entity entityPlayer, const bool facingRight); // to be called before physics update
		std::vector<vine> vines;
		entity vineClosestToPlayer;
		int nearestVineEdgeToPlayerIndex;
		glm::vec2 nearestVineEdgeToPlayerPosition;

		std::vector<entity> cigarettes;
		std::vector<entity> npcs;
		uint8_t lastCigaretteIndex = 0;
	};
}