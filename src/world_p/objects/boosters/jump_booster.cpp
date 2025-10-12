#include "jump_booster.h"
#include <flecs/flecs.h>
#include "world_p/components.h"
#include "world_p/object_components.h"
#include "world_p/transform.h"
#include "world_p/scene.h"
#include "world_p/ecs.h"
#include "renderer_p/debug/debug_draw.h"

namespace rfct {
	constexpr float boostAnimTime = 0.5f;
	static flecs::query<scaleComponent, dynamicBoxColliderComponent, jumpBoosterComponent> jumpBoosterQuery;
	void onCollision_JumpBooster_DynamicObj(entity enemy, entity collidedWith) {
		if (collidedWith.get<dynamicObjectTypeComponent>()->type != dynamicObjectType::Player) return;
		collidedWith.get_mut<velocityComponent>()->velocity.y = 3.f;
		collidedWith.get_mut<playerStateComponent>()->dashCharges = 1;
		enemy.get_mut<jumpBoosterComponent>()->timeSinceBoost = 0.f;
	}

	std::vector<Vertex> jumpBoosterVertices = {
		{{-0.17799716876470864f, 0.002142857142857141f, 0.0f}, {0.519531f, 0.339844f, 0.222656f}, 0, 0},
		{{0.3242142857142858f, 0.03736105170660148f, 0.0f}, {0.519531f, 0.339844f, 0.222656f}, 0, 0},
		{{0.3242142857142858f, 0.002142857142857141f, 0.0f}, {0.519531f, 0.339844f, 0.222656f}, 0, 0},
		{{0.3242142857142858f, 0.20928571428571427f, 0.0f}, {0.519531f, 0.339844f, 0.222656f}, 0, 0},
		{{0.3242142857142858f, 0.04958566099256981f, 0.0f}, {0.519531f, 0.339844f, 0.222656f}, 0, 0},
		{{0.002418678328599544f, 0.20928571428571427f, 0.0f}, {0.519531f, 0.339844f, 0.222656f}, 0, 0},
		{{-0.27364285714285713f, 0.20928571428571427f, 0.0f}, {0.574219f, 0.394531f, 0.277344f}, 0, 0},
		{{-0.01716867832859964f, 0.20928571428571427f, 0.0f}, {0.574219f, 0.394531f, 0.277344f}, 0, 0},
		{{0.3159171612434972f, 0.04398256809608814f, 0.0f}, {0.574219f, 0.394531f, 0.277344f}, 0, 0},
		{{-0.27364285714285713f, 0.20928571428571427f, 0.0f}, {0.574219f, 0.394531f, 0.277344f}, 0, 0},
		{{0.3159171612434972f, 0.04398256809608814f, 0.0f}, {0.574219f, 0.394531f, 0.277344f}, 0, 0},
		{{-0.27364285714285713f, 0.002638948293398481f, 0.0f}, {0.574219f, 0.394531f, 0.277344f}, 0, 0},
	};
	std::vector<Vertex> jumpBoosterVertices1 = {
		{{0.3242142857142858f, 0.20928571428571427f, 0.0f}, {0.562500f, 0.382812f, 0.265625f}, 0, 0},
		{{0.3242142857142858f, 0.04958566099256981f, 0.0f}, {0.562500f, 0.382812f, 0.265625f}, 0, 0},
		{{0.002418678328599544f, 0.20928571428571427f, 0.0f}, {0.562500f, 0.382812f, 0.265625f}, 0, 0},
		{{-0.27364285714285713f, 0.01036705584904287f, 0.0f}, {0.562500f, 0.382812f, 0.265625f}, 0, 0},
		{{-0.27364285714285713f, 0.20928571428571427f, 0.0f}, {0.562500f, 0.382812f, 0.265625f}, 0, 0},
		{{-0.026983720681384604f, 0.20928571428571427f, 0.0f}, {0.562500f, 0.382812f, 0.265625f}, 0, 0},
		{{-0.013360050370037957f, 0.2073955763410187f, 0.0f}, {0.406250f, 0.226562f, 0.109375f}, 0, 0},
		{{0.3242142857142858f, 0.03986488845797963f, 0.0f}, {0.406250f, 0.226562f, 0.109375f}, 0, 0},
		{{0.3242142857142858f, 0.002142857142857141f, 0.0f}, {0.406250f, 0.226562f, 0.109375f}, 0, 0},
		{{-0.013360050370037957f, 0.2073955763410187f, 0.0f}, {0.406250f, 0.226562f, 0.109375f}, 0, 0},
		{{0.3242142857142858f, 0.002142857142857141f, 0.0f}, {0.406250f, 0.226562f, 0.109375f}, 0, 0},
		{{-0.26787342217575827f, 0.002142857142857141f, 0.0f}, {0.406250f, 0.226562f, 0.109375f}, 0, 0},
	};
};



namespace rfct {
	void jumpBoosters::initSystem() {
		jumpBoosterQuery =
			ecs::get().query_builder<scaleComponent, dynamicBoxColliderComponent, jumpBoosterComponent>()
			.build();
	};
	void jumpBoosters::spawnData(scene* s, sceneSerializedData* sd) {
		uint8_t i = 0;
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



			entity jump = s->createDynamicRenderingEntity((i % 2) ? &jumpBoosterVertices1 : &jumpBoosterVertices, &model);
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
	};
	void jumpBoosters::resetLevel(const frameContext* ctx) {
	};
	void jumpBoosters::updateVisuals(const frameContext* ctx) {
	};
	void jumpBoosters::updateSystem(frameContext* ctx) {
		jumpBoosterQuery.each([&](flecs::entity e, scaleComponent& sc, dynamicBoxColliderComponent& box, jumpBoosterComponent& en) {
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
			});
	};
	void jumpBoosters::cleanupSystem() {
		jumpBoosterQuery.~query();
	}
}