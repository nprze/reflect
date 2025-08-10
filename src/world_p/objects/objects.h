#pragma once
#include "vine.h"
#include "npc.h"
#include "context.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"

namespace rfct{
	struct sceneSerializedData;
	constexpr uint8_t cigarettesMaxCount = 3;
	struct objectsHolder {
		objectsHolder() : cigaretteVertices(12), cigarettes(cigarettesMaxCount, entity()) {};
		void init(sceneSerializedData* serializeData, scene* parentScene);
		void update(frameContext* fc);
		void draw(const frameContext* fc);
		
		void onPlayerDash(const frameContext* fc, const entity entityPlayer, const bool facingRight); // to be called before physics update
		void constructCigaretteBoundingBox(entity cigarette);
		std::vector<vine> vines;
		entity vineClosestToPlayer;
		int nearestVineEdgeToPlayerIndex;
		glm::vec2 nearestVineEdgeToPlayerPosition;

		std::vector<entity> cigarettes;
		std::vector<Vertex> cigaretteVertices;
		std::vector<entity> npcs;
		uint8_t lastCigaretteIndex = 0;
	};
}