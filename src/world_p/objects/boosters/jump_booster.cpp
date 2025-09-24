#include "jump_booster.h"
#include <flecs/flecs.h>
#include "world_p/components.h"
#include "world_p/object_components.h"
#include "world_p/transform.h"
#include "world_p/scene.h"
#include "world_p/ecs.h"
#include "renderer_p/debug/debug_draw.h"
#include "world_p/physics/physics.h" // for daw AABB, remove when no longer debugging  that

namespace rfct {
	constexpr float boostAnimTime = .2f;
	static flecs::query<positionComponent, dynamicBoxColliderComponent, jumpBoosterComponent> jumpBoosterQuery;
	void onCollision_JumpBooster_DynamicObj(entity enemy, entity collidedWith) {
		if (collidedWith.get<dynamicObjectTypeComponent>()->type != dynamicObjectType::Player) return;
		collidedWith.get_mut<velocityComponent>()->velocity.y = 3.f;
		collidedWith.get_mut<playerStateComponent>()->dashCharges = 1;
		enemy.get_mut<jumpBoosterComponent>()->timeSinceBoost = 0.f;
	}
};

void rfct::initJumpBoosterVars(scene* parentScene, sceneSerializedData* sd)
{
	for (const JumpBoosterInfo& e : sd->boosters) {

		dynamicBoxColliderComponent bounds = {};
		bounds.min = e.min;
		bounds.max = e.max;

		const float oneSeventytieth = 1.f / 70.f;
		transform trans = {};
		trans.pos = { e.position };
		trans.scale.scale = { oneSeventytieth , oneSeventytieth };
		glm::mat4 model = getModelMatrixFromTransform(trans);
		frameContext noCtx{};

		dynamicObjCollisionCallbackComponent dynColCallback;
		dynColCallback.handler = onCollision_JumpBooster_DynamicObj;

		jumpBoosterComponent eComp = {};


		uint32_t ssboMatrixIndex = parentScene->getRenderData().addDynamicMat(&noCtx, &model);


		entity enemy = ecs::get().entity<>()
			.child_of(parentScene->sceneEntity)
			.set<dynamicSSBOIndexComponent>({ ssboMatrixIndex })
			.set<rotationComponent>({ trans.rot })
			.set<scaleComponent>({ trans.scale })
			.set<positionComponent>({ trans.pos })
			.set<gravityComponent>({ 0.97, false, 3.f })
			.set<velocityComponent>({ glm::vec2(0.f,0.f) })
			.set<dynamicBoxColliderComponent>(bounds)
			.set<dynamicObjCollisionCallbackComponent>(dynColCallback)
			.set<jumpBoosterComponent>(eComp)
			.set<dynamicObjectTypeComponent>({ dynamicObjectType::JumpBooster, false });

	}
	jumpBoosterQuery =
		ecs::get().query_builder<positionComponent, dynamicBoxColliderComponent, jumpBoosterComponent>()
		.with(flecs::ChildOf, parentScene->sceneEntity)
		.build();
}

void rfct::updateJumpBoosters(frameContext* ctx)
{
	jumpBoosterQuery.each([&](flecs::entity e, positionComponent& pos, dynamicBoxColliderComponent& box , jumpBoosterComponent& en) {
		glm::vec2 heightPlus = { 0,0 };
		en.timeSinceBoost += (en.timeSinceBoost == -1.f?0.f:1.f) * ctx->fixedUpdateTimes * fixedDeltaTime;
		if (en.timeSinceBoost >= boostAnimTime) {
			en.timeSinceBoost = -1.f;
		}
		if (en.timeSinceBoost >= 0) {
			heightPlus.y += -std::pow(en.timeSinceBoost * (1/ boostAnimTime), 2) + 1;
		}
		drawAABB(pos.position + box.min, pos.position + box.max + heightPlus, 1);
		});
}

void rfct::cleanupJumpBoosterVars()
{
	jumpBoosterQuery.~query();
}
