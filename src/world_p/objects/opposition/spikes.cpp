#include "spikes.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"
#include "world_p/scene.h"
#include "world_p/object_components.h"
#include "world_p/components.h"


namespace rfct {
	std::vector<Vertex> vertices; // doing this to avoid smaller alloc
}
namespace rfct {
	void onCollision_Spike_DynamicObj(entity vineEntity, entity collidedWith) {
		
		if (ecs::get().get<dynamicObjectTypeComponent>(collidedWith).type != dynamicObjectType::Player) return;
		ecs::get().get<playerLifeComponent>(collidedWith).alive = false;
	}
}

namespace rfct {
	void spikes::initSystem() {
		constexpr size_t maxSpikesInOneLine = 100;
		vertices.resize(maxSpikesInOneLine * 3);
	};
	void spikes::spawnData(scene* s, sceneSerializedData* sd) {
		for (SpikeInfo& spawnInfo : sd->spikes) {
			// spikes along the line defined by spawnInfo min and max, a spike size is 0.5f x 0.5f.
			glm::vec2 alongTheSpawn = glm::normalize(spawnInfo.max - spawnInfo.min);
			glm::vec2 perpenicular = getMoveDir(spawnInfo.dir);
			const uint32_t numSpikes = glm::length(spawnInfo.max - spawnInfo.min) * 4;
			glm::vec3 baseColor;
			float fluc = 0;
			for (uint32_t i = 0; i < numSpikes; ++i) {
				fluc = randF();
				baseColor = glm::vec3(
					0.4f + (0.2f * fluc),
					0.4f + (0.2f * fluc),
					0.4f + (0.2f * fluc)
				);
				vertices[i * 3 + 0].color = baseColor;
				vertices[i * 3 + 1].color = baseColor;
				vertices[i * 3 + 2].color = baseColor;

				vertices[i * 3 + 0].pos = glm::vec3(spawnInfo.min + (alongTheSpawn * (float)i * 0.25f), 0.f);
				vertices[i * 3 + 1].pos = glm::vec3(spawnInfo.min + (alongTheSpawn * (float)(i + 1) * 0.25f), 0.f);
				vertices[i * 3 + 2].pos = glm::vec3(spawnInfo.min + ((alongTheSpawn * (float)(2 * i + 1) * 0.125f) + (perpenicular * 0.3f)), 0.f);
			}
			glm::mat4 modelMat = glm::mat4(1);
			entity spikeEntity = s->createDynamicRenderingEntity(&vertices, &modelMat, glm::length(spawnInfo.max - spawnInfo.min) * 4 * 3);
			dynamicBoxColliderComponent boc;
			spawnInfo.max += (perpenicular * 0.2f);
			boc.min = glm::vec2(std::min(spawnInfo.min.x, spawnInfo.max.x), std::min(spawnInfo.min.y, spawnInfo.max.y));
			boc.max = glm::vec2(std::max(spawnInfo.min.x, spawnInfo.max.x), std::max(spawnInfo.min.y, spawnInfo.max.y));
			boc.max -= alongTheSpawn * 0.05f;
			boc.min += alongTheSpawn * 0.05f;

			entt::registry& reg = ecs::get();

			reg.emplace_or_replace<dynamicBoxColliderComponent>(spikeEntity, boc);
			reg.emplace_or_replace<dynamicObjectTypeComponent>(spikeEntity, dynamicObjectTypeComponent{ dynamicObjectType::Spike });
			dynamicObjCollisionCallbackComponent dynColCallback;
			dynColCallback.handler = onCollision_Spike_DynamicObj;
			reg.emplace_or_replace<dynamicObjCollisionCallbackComponent>(spikeEntity, dynColCallback);
		}
	};
}
