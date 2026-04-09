#include "grass.h"
#include <glm/glm.hpp>
#include "renderer_p/rasterizer_pipeline/vertex.h"
#include "world_p/components.h"
#include "world_p/object_components.h"
#include "world_p/transform.h"
#include "assets/serialize_structures/scene_serialize_data.h"
#include "world_p/scene.h"
#include "world_p/player/player.h"

namespace rfct {
	std::vector<Vertex> grassVertices = {
	   {{-0.00088f, 0.264f, 0.0f}, {0.55f, 0.80f, 0.30f}, 0, 0},
	   {{-0.10956f, 0.000f, 0.0f}, {0.55f, 0.80f, 0.30f}, 0, 0},
	   {{ 0.10956f, 0.000f, 0.0f}, {0.55f, 0.80f, 0.30f}, 0, 0},

	   {{-0.00088f, 0.264f, 0.0f}, {0.55f, 0.80f, 0.30f}, 0, 0},
	   {{ 0.10956f, 0.000f, 0.0f}, {0.55f, 0.80f, 0.30f}, 0, 0},
	   {{ 0.00000f, 0.420f, 0.0f}, {0.55f, 0.80f, 0.30f}, 0, 0},

	   {{ 0.14608f, 0.000f, 0.0f}, {0.00f, 0.58f, 0.27f}, 0, 0},
	   {{ 0.10956f, 0.166f, 0.0f}, {0.00f, 0.58f, 0.27f}, 0, 0},
	   {{ 0.22000f, 0.346f, 0.0f}, {0.00f, 0.58f, 0.27f}, 0, 0},

	   {{-0.06688f, 0.196f, 0.0f}, {0.22f, 0.70f, 0.33f}, 0, 0},
	   {{-0.22000f, 0.360f, 0.0f}, {0.22f, 0.70f, 0.33f}, 0, 0},
	   {{-0.18304f, 0.000f, 0.0f}, {0.22f, 0.70f, 0.33f}, 0, 0},

	   {{-0.06688f, 0.196f, 0.0f}, {0.22f, 0.70f, 0.33f}, 0, 0},
	   {{-0.18304f, 0.000f, 0.0f}, {0.22f, 0.70f, 0.33f}, 0, 0},
	   {{-0.13288f, 0.000f, 0.0f}, {0.22f, 0.70f, 0.33f}, 0, 0},
	};

	void onCollision_TallGrass_DynamicObj(entity grassEnt, entity collidedWith) {
		entt::registry& reg = ecs::get();
		if (reg.get<dynamicObjectTypeComponent>(collidedWith).type != dynamicObjectType::Player) return;
		reg.get<grassComponent>(grassEnt).beenTouched = true;
	};
};

void rfct::spawnTallGrass(scene* parentScene, sceneSerializedData* sd) {
	entt::registry& reg = ecs::get();
	for (const TallGrassInfo& e : sd->tallGrass) {
		dynamicBoxColliderComponent bounds = {};
		bounds.min = { -0.3, 0.0 };
		bounds.max = { 0.3, 0.2 };

		transform trans = {};
		trans.pos = { e.position };
		glm::mat4 model = getModelMatrixFromTransform(trans);

		dynamicObjCollisionCallbackComponent dynColCallback;
		dynColCallback.handler = onCollision_TallGrass_DynamicObj;

		grassComponent eComp = {};

		entity jump = parentScene->createDynamicRenderingEntity(&grassVertices, &model);

		reg.emplace_or_replace<rotationComponent>(jump, rotationComponent{ trans.rot });
		reg.emplace_or_replace<scaleComponent>(jump, scaleComponent{ trans.scale });
		reg.emplace_or_replace<positionComponent>(jump, positionComponent{ trans.pos });
		reg.emplace_or_replace<gravityComponent>(jump, gravityComponent{ 0.97, false, 3.f });
		reg.emplace_or_replace<velocityComponent>(jump, velocityComponent{ glm::vec2(0.f, 0.f) });
		reg.emplace_or_replace<dynamicBoxColliderComponent>(jump, bounds);
		reg.emplace_or_replace<dynamicObjCollisionCallbackComponent>(jump, dynColCallback);
		reg.emplace_or_replace<grassComponent>(jump, eComp);
		reg.emplace_or_replace<dynamicObjectTypeComponent>(jump, dynamicObjectTypeComponent{ dynamicObjectType::JumpBooster, false });
	}
}

void rfct::updateGrass(frameContext* ctx)
{
	auto tallGrassQuery = ecs::get().view<grassComponent, positionComponent, scaleComponent, rotationComponent>();

	float currentMultiplier = playerController::get().facingRight ? -1.f : 1.f;

	for (auto [ent, gc, pos, sc, rot] : tallGrassQuery.each()) {
		if (gc.beenTouched) {
			if (gc.canBeFirst && gc.timeSinceTouched <= 0.5f) {
				gc.canBeFirst = false;
				gc.timeSinceTouched = 1.f;
				gc.sign = currentMultiplier;
			}
		}
		else {
			gc.canBeFirst = true;
		}
		gc.timeSinceTouched = std::clamp(gc.timeSinceTouched - ctx->dt, 0.f, 1.f);
		gc.beenTouched = false;

		float x = std::clamp(1.f - gc.timeSinceTouched, 0.f, 1.f);
		rot.rotation.z = gc.sign * 0.3 * glm::cos(3.5f * glm::pi<float>() * x) * (1.f - x);

		ctx->scene->updateTransformData(ctx, ent);
	}
}