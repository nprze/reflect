#include "npc.h"
#include "world_p/objects/objects.h"
#include "context.h"
#include "world_p/scene.h"
#include "world_p/object_components.h"
#include "renderer_p/debug/debug_draw.h"
#include "input.h"

#include "world_p/dialogue/dialogue.h"

float distanceSquared(const glm::vec2* pos1, const glm::vec2* pos2) {
	return glm::dot(*pos1 - *pos2, *pos1 - *pos2);
}
void rfct::updateNpc(frameContext* ctx, entity npcEntity) {
}


namespace rfct {
	std::vector<entity> npcsVec;

	dialogue* m_currentlyPlayingDialogue = nullptr;
}

namespace rfct {
	void npcs::spawnData(scene* s, sceneSerializedData* sd) {
		for (NPCInfo npcInfo : sd->npcs) {
			dynamicBoxColliderComponent boc = { npcInfo.min, npcInfo.max };
			entity npcEntity = s->createDynamicRect(&boc);
			npcEntity.set<dynamicObjectTypeComponent>({ dynamicObjectType::NPC });
			npcEntity.set<interactionDistanceComponent>({ npcInfo.interatcionRadius * npcInfo.interatcionRadius });
			npcEntity.set<dialoguePathComponent>({ npcInfo.dialogueFile });
			npcEntity.set<positionComponent>({ (boc.min + boc.max) * 0.5f });
			npcsVec.push_back(npcEntity);
		}
	};
	void npcs::resetLevel(const frameContext* ctx) {
	};
	void npcs::updateVisuals(const frameContext* ctx) {
	};
	void npcs::updateSystem(frameContext* ctx) {
		if (ctx->state == gameState::gameplay) {
			for (entity& npcEntity : npcsVec) {
				if (ctx->fixedUpdateTimes) {
					float dist = distanceSquared(&(ctx->scene->getPlayer()).get<positionComponent>()->position, &npcEntity.get<positionComponent>()->position);
					float wantedDist = npcEntity.get<interactionDistanceComponent>()->interationDistanceSquared;
					if (dist <= wantedDist) {
						if (input::getInput().hold && ctx->scene->getPlayer().get<playerStateComponent>()->grounded) {
							ctx->state = gameState::stateDialogue;
							startDialogue(npcEntity.get<dialoguePathComponent>()->dialoguePath);
						}
						else {
							debugDraw::drawText("talk to me", { 100.f, 100.f }, .3f);
						}
					}
				}
			}
		}

		if (ctx->state == gameState::stateDialogue) {
			if (m_currentlyPlayingDialogue->update(ctx)) {
				delete m_currentlyPlayingDialogue;
				m_currentlyPlayingDialogue = nullptr;
				ctx->state = gameState::gameplay;
			}
		}

	};
	void npcs::cleanupSystem() {
		if (m_currentlyPlayingDialogue) {
			delete m_currentlyPlayingDialogue;
		}
	}
	void npcs::startDialogue(const std::string& path)
	{
		m_currentlyPlayingDialogue = new dialogue(path);
		m_currentlyPlayingDialogue->fullLoad();
	}
}
