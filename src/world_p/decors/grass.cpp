#include "grass.h"
#include <glm/glm.hpp>
#include "renderer_p/rasterizer_pipeline/vertex.h"
#include "world_p/components.h"
#include "world_p/object_components.h"
#include "world_p/ecs.h"
#include "world_p/transform.h"
#include "assets/serialize_structures/scene_serialize_data.h"
#include "world_p/scene.h"

namespace rfct
{
	std::vector<Vertex> grassVertices = {
	{{-0.22f, 0.00f, 0.0f}, {0.15f, 0.40f, 0.10f}, 0, 0},
	{{-0.18f, 0.00f, 0.0f}, {0.15f, 0.40f, 0.10f}, 0, 0},
	{{-0.20f, 0.18f, 0.0f}, {0.20f, 0.60f, 0.18f}, 0, 0},
	{{-0.18f, 0.00f, 0.0f}, {0.15f, 0.40f, 0.10f}, 0, 0},
	{{-0.16f, 0.18f, 0.0f}, {0.22f, 0.63f, 0.20f}, 0, 0},
	{{-0.20f, 0.18f, 0.0f}, {0.20f, 0.60f, 0.18f}, 0, 0},

	{{-0.02f, 0.00f, 0.0f}, {0.20f, 0.55f, 0.12f}, 0, 0},
	{{ 0.02f, 0.00f, 0.0f}, {0.20f, 0.55f, 0.12f}, 0, 0},
	{{ 0.03f, 0.21f, 0.0f}, {0.28f, 0.75f, 0.25f}, 0, 0},
	{{ 0.02f, 0.00f, 0.0f}, {0.20f, 0.55f, 0.12f}, 0, 0},
	{{ 0.06f, 0.20f, 0.0f}, {0.30f, 0.78f, 0.30f}, 0, 0},
	{{ 0.03f, 0.21f, 0.0f}, {0.28f, 0.75f, 0.25f}, 0, 0},

	{{ 0.18f, 0.00f, 0.0f}, {0.30f, 0.60f, 0.10f}, 0, 0},
	{{ 0.22f, 0.00f, 0.0f}, {0.30f, 0.60f, 0.10f}, 0, 0},
	{{ 0.17f, 0.19f, 0.0f}, {0.40f, 0.80f, 0.25f}, 0, 0},
	{{ 0.22f, 0.00f, 0.0f}, {0.30f, 0.60f, 0.10f}, 0, 0},
	{{ 0.20f, 0.20f, 0.0f}, {0.45f, 0.82f, 0.28f}, 0, 0},
	{{ 0.17f, 0.19f, 0.0f}, {0.40f, 0.80f, 0.25f}, 0, 0},
	};

	void onCollision_TallGrass_DynamicObj(entity grassEnt, entity collidedWith) {
		entt::registry& reg = ecs::get();
		if (reg.get<dynamicObjectTypeComponent>(collidedWith).type != dynamicObjectType::Player) return;

		reg.get<grassComponent>(grassEnt).beenTouched = true;
	};
};

void rfct::initGrassVars(scene* parentScene)
{
}

void rfct::spawnTallGrass(scene* parentScene, sceneSerializedData* sd)
{
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

void rfct::cleanupGrass()
{
}

void rfct::updateGrass(frameContext* ctx)
{
	auto tallGrassQuery = ecs::get().view<grassComponent, positionComponent, scaleComponent, rotationComponent>();

	for (auto [ent, gc, pos, sc, rot] : tallGrassQuery.each()) {
		if (gc.beenTouched) {
			if (gc.canBeFirst) {
				gc.canBeFirst = false;
				gc.timeSinceTouched = 1.f;
			}
		}
		else {
			gc.canBeFirst = true;
		}
		float x = std::clamp(1.f - gc.timeSinceTouched, 0.f, 1.f);
		float swayAmount = 90 * glm::cos(3.5f * glm::pi<float>() * x);//* (1.f - x) * 15.f; // sway decreases over time
		rot.rotation.z = glm::radians(swayAmount);


		//if (gc.beenTouched) {
		//	gc.timeSinceTouched += ctx->dt;
		//	if (gc.timeSinceTouched >= 1.f) {
		//		gc.beenTouched = false;
		//		gc.timeSinceTouched = 0.f;
		//	} else {
		//		// simple sway animation
		//		float swayAmount = glm::sin(gc.timeSinceTouched * 10.f) * (1.f - (gc.timeSinceTouched / 1.f)) * 15.f; // sway decreases over time
		//		rot.rotation.z = glm::radians(swayAmount);
		//	}
		//} else {
		//	rot.rotation.z = 0.f; // reset rotation when not touched
		//}
		////gc.beenTouched = false;



		ctx->scene->updateTransformData(ctx, ent);
	}
}