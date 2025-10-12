#include "dash_recharge.h"
#include <flecs/flecs.h>
#include "world_p/components.h"
#include "world_p/object_components.h"
#include "world_p/transform.h"
#include "world_p/scene.h"
#include "world_p/ecs.h"
#include "renderer_p/debug/debug_draw.h"
#include "world_p/physics/physics.h" // for daw AABB, remove when no longer debugging  that

namespace rfct {
	/*
	constexpr float boostAnimTime = 0.5f;
	static flecs::query<scaleComponent, dynamicBoxColliderComponent, jumpBoosterComponent> jumpBoosterQuery;
	void onCollision_JumpBooster_DynamicObj(entity enemy, entity collidedWith) {
		if (collidedWith.get<dynamicObjectTypeComponent>()->type != dynamicObjectType::Player) return;
		collidedWith.get_mut<velocityComponent>()->velocity.y = 3.f;
		collidedWith.get_mut<playerStateComponent>()->dashCharges = 1;
		enemy.get_mut<jumpBoosterComponent>()->timeSinceBoost = 0.f;
	}
	*/
};

void rfct::initDashRechargeVars(scene* parentScene, sceneSerializedData* sd)
{
	/*uint8_t i = 1;
	for (const JumpBoosterInfo& e : sd->boosters) {

		dynamicBoxColliderComponent bounds = {};
		bounds.min = { -0.3, 0.0 };
		bounds.max = { 0.3, 0.2 };

		transform trans = {};
		trans.pos = { e.position };
		glm::mat4 model = getModelMatrixFromTransform(trans);
		frameContext noCtx{};

		dynamicObjCollisionCallbackComponent dynColCallback;
		dynColCallback.handler = onCollision_JumpBooster_DynamicObj;

		jumpBoosterComponent eComp = {};



		entity jump = parentScene->createDynamicRenderingEntity((i % 2) ? &jumpBoosterVertices1 : &jumpBoosterVertices, &model);
		jump
			.set<rotationComponent>({ trans.rot })
			.set<scaleComponent>({ trans.scale })
			.set<positionComponent>({ trans.pos })
			.set<gravityComponent>({ 0.97, false, 3.f })
			.set<velocityComponent>({ glm::vec2(0.f,0.f) })
			.set<dynamicBoxColliderComponent>(bounds)
			.set<dynamicObjCollisionCallbackComponent>(dynColCallback)
			.set<jumpBoosterComponent>(eComp)
			.set<dynamicObjectTypeComponent>({ dynamicObjectType::JumpBooster, false });
		i++;
	}
	jumpBoosterQuery =
		ecs::get().query_builder<scaleComponent, dynamicBoxColliderComponent, jumpBoosterComponent>()
		.with(flecs::ChildOf, parentScene->sceneEntity)
		.build();*/
}

void rfct::updateDashRecharges(frameContext* ctx)
{
	/*jumpBoosterQuery.each([&](flecs::entity e, scaleComponent& sc, dynamicBoxColliderComponent& box, jumpBoosterComponent& en) {
		glm::vec2 heightPlus = { 0,0 };
		en.timeSinceBoost += (en.timeSinceBoost == -1.f ? 0.f : 1.f) * ctx->fixedUpdateTimes * fixedDeltaTime;
		if (en.timeSinceBoost >= boostAnimTime) {
			en.timeSinceBoost = -1.f;
		}
		if (en.timeSinceBoost >= 0) {
			heightPlus.y += -std::pow(en.timeSinceBoost * (1 / boostAnimTime), 2) + 1;
		}

		sc.scale.y = 1.0f + heightPlus.y;
		sc.scale.x = 1.0f - (heightPlus.y * 0.3);

		ctx->scene->updateTransformData(ctx, e);
		});*/
}

void rfct::cleanupDashRechargeVars()
{
	//jumpBoosterQuery.~query();
}
