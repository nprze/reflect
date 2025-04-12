#include "components.h"
#include "scene.h"
#include "transform.h"
#include "input.h"
#include "flecs\flecs.h"
#include "ecs.h"
namespace rfct {
	void registerComponents()
	{
		ecs::get().component<sceneComponent>();
		ecs::get().component<cameraComponent>();
		ecs::get().component<positionComponent>();
		ecs::get().component<rotationComponent>();
		ecs::get().component<scaleComponent>();
		ecs::get().component<matrixComponent>();
		ecs::get().component<staticSSBOIndexComponent>();
		ecs::get().component<dynamicSSBOIndexComponent>();
		ecs::get().component<vertexRenderInfoComponent>();
		ecs::get().component<staticBoxColliderComponent>();
		ecs::get().component<dynamicBoxColliderComponent>();
		ecs::get().component<gravityComponent>();
		ecs::get().component<velocityComponent>();

		

	}

	void updateDynamicRenderMeshComponents(scene* scene, frameContext* ctx) {
		/*
		std::unordered_map<ComponentEnum, std::vector<void*>> out_components;
		scene->getAllComponents(out_components, ComponentEnum::dynamicRenderMeshComponent | ComponentEnum::transformComponent);
		std::vector<void*> rmc = out_components[ComponentEnum::dynamicRenderMeshComponent];
		std::vector<void*> tc = out_components[ComponentEnum::transformComponent];
		for (size_t num_vector_of_vectors = 0; num_vector_of_vectors < rmc.size(); num_vector_of_vectors++) { // each archetype has a separate vector of components hence the nesting
			std::vector<dynamicRenderMeshComponent>* RenderMeshComponents = static_cast<std::vector<dynamicRenderMeshComponent>*>(rmc[num_vector_of_vectors]);
			std::vector<transformComponent>* TransformComponents = static_cast<std::vector<transformComponent>*>(tc[num_vector_of_vectors]);
			for (size_t i = 0; i < RenderMeshComponents->size();i++) {
				glm::mat4 model = getModelMatrixFromEntity(TransformComponents->at(i));
				scene->getRenderData().updateMat(ctx, RenderMeshComponents->at(i).renderDataLocations, &model);
			}
		}*/
	}
	void updatePlayer(scene* scene, flecs::entity* player, float dt)
	{
		/*
		transformComponent* tc = scene->getComponent<transformComponent>(player);
		if (input::getInput().xAxis) {
			tc->position.x += 3 * input::getInput().xAxis * dt;
		}
		if (input::getInput().yAxis) {
			tc->position.y += 3 * input::getInput().yAxis * dt;
		}*/
	}


	struct collision {

		dynamicBoxColliderComponent* obj_a;
		staticBoxColliderComponent* obj_b;
	};
	void updatePhysics(scene* scene, float dt)
	{
		std::vector<collision> collisions;
		collisions.reserve(3);
		const float fixedDelta = 1.0f / 60.0f;
		static float accumulator = 0.f;
		accumulator += dt;
		if (accumulator < fixedDelta) return;

	}


	bool checkForCollisionAABBAABB(dynamicBoxColliderComponent* a, staticBoxColliderComponent* b)
	{
		if (a->min.x > b->max.x || a->max.x < b->min.x) return false;
		if (a->min.y > b->max.y || a->max.y < b->min.y) return false;
		if (a->min.z > b->max.z || a->max.z < b->min.z) return false;
		return true;
	}
}