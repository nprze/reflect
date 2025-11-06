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

	dialogue* m_currentlyPlayingDialogue = nullptr;
}

namespace rfct {
    void npcs::spawnData(scene* s, sceneSerializedData* sd) {
        auto& reg = ecs::get();

        for (const NPCInfo& npcInfo : sd->npcs) {
            dynamicBoxColliderComponent boc = { npcInfo.min, npcInfo.max };

            entt::entity npcEntity = s->createDynamicRect(&boc);

            reg.emplace<dynamicObjectTypeComponent>(npcEntity, dynamicObjectType::NPC);
            reg.emplace<interactionDistanceComponent>(
                npcEntity, npcInfo.interatcionRadius * npcInfo.interatcionRadius
            );
            reg.emplace<dialoguePathComponent>(npcEntity, npcInfo.dialogueFile);
            reg.emplace_or_replace<positionComponent>(npcEntity, (boc.min + boc.max) * 0.5f);
        }
    };

    void npcs::resetLevel(const frameContext* ctx) {
    }/*
    void npcs::onLevelSwitch(scene* scen)
    {
        auto vineQuery = ecs::get().view<dialoguePathComponent>();
        for (auto [ent, sc] : vineQuery.each()) {
            scen->deleteDynamicEntity(ent);
        }
    }
    ;*/

    void npcs::updateVisuals(const frameContext* ctx) {
    };

    void npcs::updateSystem(frameContext* ctx) {
        auto& reg = ecs::get();

        if (ctx->state == gameState::gameplay) {
            auto gravityVelocityPositionBoxQuery = ecs::get().view<positionComponent, interactionDistanceComponent, dialoguePathComponent>();
            for (auto [npcEntity, npcPos, inter, dial] : gravityVelocityPositionBoxQuery.each()) {
                if (ctx->fixedUpdateTimes) {
                    const auto& playerPos = reg.get<positionComponent>(ctx->scene->getPlayer());

                    float dist = distanceSquared(&playerPos.position, &npcPos.position);

                    float wantedDist = inter.interationDistanceSquared;

                    if (dist <= wantedDist) {
                        const auto& playerState = reg.get<playerStateComponent>(ctx->scene->getPlayer());

                        if (input::getInput().hold && playerState.grounded) {
                            ctx->state = gameState::stateDialogue;

                            startDialogue(dial.dialoguePath);
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
