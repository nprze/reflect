#include "npc.h"
#include "input.h"
#include "renderer_p/debug/debug_draw.h"
#include "world_p/objects/objects.h"
#include "world_p/scene.h"
#include "world_p/object_components.h"
#include "world_p/dialogue/dialogue.h"
#include "world_p/camera/camera.h"
#include "renderer_p/renderer.h"

rfct::dialogue* m_currentlyPlayingDialogue = nullptr;
bool talkPopupVisible = false;
glm::vec2 talkPopupPosition = { 0.f, 0.f };

float distanceSquared(const glm::vec2* pos1, const glm::vec2* pos2) {
	return glm::dot(*pos1 - *pos2, *pos1 - *pos2);
}

namespace rfct {
    void npcs::spawnData(scene* s, sceneSerializedData* sd) {
        RFCT_PROFILE_FUNCTION();
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

    void npcs::updateVisuals(const frameContext* ctx) {
        if (m_currentlyPlayingDialogue != nullptr) {
			m_currentlyPlayingDialogue->visualUpdate(ctx);
        }
        if (!talkPopupVisible) return;
        glm::vec2 screenSpaceTextPos = getVPMatrix() * glm::vec4(talkPopupPosition.x, talkPopupPosition.y + 2.f, 0.f, 1.f);
        vk::Extent2D winExtent = renderer::getRen().getWindow().getExtent();
		glm::vec2 appScreenSpacePos = (screenSpaceTextPos + glm::vec2(1.f, 1.f)) * 0.5f * glm::vec2(winExtent.width, winExtent.height);
        font* defaultFont = renderer::getRen().getUIPipeline().getDefaultFont();
        float width = defaultFont->getTextWidth("TALK", 0.055f * winExtent.height) * 0.5f;
		appScreenSpacePos.x -= width;
        renderer::getRen().getUIPipeline().addTextVerticesHeight(std::string("TALK"), appScreenSpacePos, 0.04f * winExtent.height, glm::vec3(1.f, 1.f, 1.f));
    }

    void npcs::updateSystem(frameContext* ctx) {
        RFCT_PROFILE_FUNCTION();
        auto& reg = ecs::get();

        if (ctx->state == gameState::gameplay) {
			float nearestDistanceSqared = FLT_MAX;
			entity nearestNPC = entt::null;
            auto gravityVelocityPositionBoxQuery = ecs::get().view<positionComponent, interactionDistanceComponent, dialoguePathComponent>();
            for (auto [npcEntity, npcPos, inter, dial] : gravityVelocityPositionBoxQuery.each()) {
                const auto& playerPos = reg.get<positionComponent>(ctx->scene->getPlayer());

                float dist = distanceSquared(&playerPos.position, &npcPos.position);

                float wantedDist = inter.interationDistanceSquared;

                if (dist <= wantedDist) {
                    if (dist < nearestDistanceSqared) {
                        nearestDistanceSqared = dist;
                        nearestNPC = npcEntity;
                    }
                }
            }
			// take only the nearest npc into consideration
            talkPopupVisible = false;
            if (nearestDistanceSqared != FLT_MAX) {
                const auto& playerState = reg.get<playerStateComponent>(ctx->scene->getPlayer());
                if (input::getInput().hold && playerState.state == playerState::normal) {
                    ctx->state = gameState::stateDialogue;
                    startDialogue(reg.get<dialoguePathComponent>(nearestNPC).dialoguePath);
                    talkPopupVisible = false;
                }
                else {
                    talkPopupVisible = true;
					talkPopupPosition = reg.get<positionComponent>(nearestNPC).position;
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

	void npcs::startDialogue(const std::string& path) {
		m_currentlyPlayingDialogue = new dialogue(path);
		m_currentlyPlayingDialogue->fullLoad();
	}
}
