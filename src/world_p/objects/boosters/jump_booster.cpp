#include "jump_booster.h"
#include "world_p/components.h"
#include "world_p/object_components.h"
#include "world_p/transform.h"
#include "world_p/scene.h"
#include "renderer_p/debug/debug_draw.h"

namespace rfct {
	constexpr float boostAnimTime = 0.5f;

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
	void onCollision_JumpBooster_DynamicObj(entity enemy, entity collidedWith) {
		entt::registry& reg = ecs::get();
		if (reg.get<dynamicObjectTypeComponent>(collidedWith).type != dynamicObjectType::Player) return;

		reg.get<velocityComponent>(collidedWith).velocity.y = 3.6f;
		reg.get<playerStateComponent>(collidedWith).dashCharges = 1;
		reg.get<jumpBoosterComponent>(enemy).timeSinceBoost = 0.0f;
	}

	void jumpBoosters::spawnData(scene* s, sceneSerializedData* sd) {
		RFCT_PROFILE_FUNCTION();
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

			entt::registry& reg = ecs::get();
			entity jump = s->createDynamicRenderingEntity((i % 2) ? &jumpBoosterVertices1 : &jumpBoosterVertices, &model);

			reg.emplace_or_replace<rotationComponent>(jump, rotationComponent{ trans.rot });
			reg.emplace_or_replace<scaleComponent>(jump, scaleComponent{ trans.scale });
			reg.emplace_or_replace<positionComponent>(jump, positionComponent{ trans.pos });
			reg.emplace_or_replace<gravityComponent>(jump, gravityComponent{ 0.97, false, 3.f });
			reg.emplace_or_replace<velocityComponent>(jump, velocityComponent{ glm::vec2(0.f, 0.f) });
			reg.emplace_or_replace<dynamicBoxColliderComponent>(jump, bounds);
			reg.emplace_or_replace<dynamicObjCollisionCallbackComponent>(jump, dynColCallback);
			reg.emplace_or_replace<jumpBoosterComponent>(jump, eComp);
			reg.emplace_or_replace<dynamicObjectTypeComponent>(jump, dynamicObjectTypeComponent{ dynamicObjectType::JumpBooster, false });

			i++;
		}
	};

	void jumpBoosters::updateSystem(frameContext* ctx) { 
		RFCT_PROFILE_FUNCTION();
		auto jumpBoosterQuery = ecs::get().view<scaleComponent, dynamicBoxColliderComponent, jumpBoosterComponent, dynamicSSBOIndexComponent>();
		for (auto [ent, sc, box, en, i] : jumpBoosterQuery.each()) {
			glm::vec2 heightPlus = { 0,0 };
			en.timeSinceBoost += (en.timeSinceBoost == -1.f ? 0.f : 1.f) * fixedDeltaTime;
			if (en.timeSinceBoost >= boostAnimTime) {
				en.timeSinceBoost = -1.f;
			}
			if (en.timeSinceBoost >= 0) {
				heightPlus.y += -std::pow(en.timeSinceBoost * (1 / boostAnimTime), 2) + 1;
			}

			sc.scale.y = 1.0f + heightPlus.y;
			sc.scale.x = 1.0f - (heightPlus.y * 0.3);

			ctx->scene->updateTransformData(ctx, ent);
		};
	};
}