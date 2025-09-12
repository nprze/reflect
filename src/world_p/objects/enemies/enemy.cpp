#include "enemy.h"
#include "assets/scene_serialize_data.h"
#include "world_p/ecs.h"
#include "world_p/components.h"
#include "world_p/transform.h"
#include "world_p/scene.h"

namespace rfct {
	static std::vector<entity> enemiesVec;
	void onCollision_Enemy_StaticObj(entity enemy, entity collidedWith, glm::vec2 resolution) {
		const positionComponent* pos = enemy.get<positionComponent>();
		//RFCT_INFO("pos: ({}, {})", pos->position.x, pos->position.y);
		RFCT_INFO("resolution: ({}, {})", resolution.x, resolution.y);
		return;
	}
};

void rfct::spawnEnemies(sceneSerializedData* sc, scene* parent)
{
	enemiesVec.reserve(sc->enemies.size());
	for (const BasicEnemyInfo& e : sc->enemies) {
		std::vector<Vertex> vertices(6);
		for (uint8_t i = 0; i < 6; ++i) {
			vertices[i].color = glm::vec3(0.8f, 0.2f, 0.7f);
		}
		vertices[0].pos = { e.min.x , e.min.y, 0.f };
		vertices[1].pos = { e.min.x , e.max.y, 0.f };
		vertices[2].pos = { e.max.x , e.min.y, 0.f };

		vertices[3].pos = { e.min.x , e.max.y, 0.f };
		vertices[4].pos = { e.max.x , e.min.y, 0.f };
		vertices[5].pos = { e.max.x , e.max.y, 0.f };

		dynamicBoxColliderComponent bounds = {};
		bounds.min = e.min;
		bounds.max = e.max;

		transform trans = {};
		trans.pos = { e.position };
		glm::mat4 model = getModelMatrixFromTransform(trans);
		frameContext noCtx{}; 

		staticObjCollisionCallbackComponent colCallback;
		colCallback.handler = onCollision_Enemy_StaticObj;
		entity enemy = parent->createDynamicRenderingEntity(&vertices, &model);
		enemy.set<rotationComponent>({ trans.rot })
			.set<scaleComponent>({ trans.scale })
			.set<positionComponent>({ trans.pos })
			.set<gravityComponent>({0.99, true, .1f})
			.set<velocityComponent>({ glm::vec2(0.f,0.f) })
			.set<dynamicBoxColliderComponent>(bounds)
			.set<staticObjCollisionCallbackComponent>(colCallback)
			.set<dynamicObjectTypeComponent>({ dynamicObjectType::Enemy });
		enemiesVec.push_back(enemy);
	}
}

void rfct::updateEnemies(frameContext* ctx)
{
	for (entity e : enemiesVec) {
		ctx->scene->updateTransformData(ctx, e);
	}
}
