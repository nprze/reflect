#include "npc.h"
#include "objects.h"
#include "context.h"
#include "world_p/scene.h"
#include "world_p/object_components.h"
#include "renderer_p/debug/debug_draw.h"
#include "input.h"


float distanceSquared(const glm::vec2* pos1, const glm::vec2* pos2) {
	return glm::dot(*pos1 - *pos2, *pos1 - *pos2);
}
void rfct::updateNpc(frameContext* ctx, entity npcEntity, objectsHolder* owner) {
	if (ctx->state == gameState::gameplay) {
		if (ctx->fixedUpdateTimes) {
			float dist = distanceSquared(&(ctx->scene->getPlayer()).get<positionComponent>()->position, &npcEntity.get<positionComponent>()->position);
			float wantedDist = npcEntity.get<interactionDistanceComponent>()->inetrationDistanceSquared;
			if (dist <= wantedDist) {
				if (input::getInput().hold) {
					ctx->scene->startDialogue(npcEntity.get<dialoguePathComponent>()->dialoguePath, ctx);
				}
				debugDraw::drawText("talk to me", { 100.f, 100.f }, .3f);
			}
		}
	}
}