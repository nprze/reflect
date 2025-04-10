#include "components.h"
#include "scene.h"
#include "transform.h"
#include "input.h"
namespace rfct {
	void updateDynamicRenderMeshComponents(scene* scene, frameContext* ctx) {
		std::unordered_map<ComponentEnum, std::vector<void*>> out_components;
		scene->getAllComponents(out_components, ComponentEnum::dynamicRenderMeshComponent | ComponentEnum::transformComponent);
		std::vector<void*> rmc = out_components[ComponentEnum::dynamicRenderMeshComponent];
		std::vector<void*> tc = out_components[ComponentEnum::transformComponent];
		for (size_t num_vector_of_vectors = 0; num_vector_of_vectors < rmc.size(); num_vector_of_vectors++) { // each archetype has a separate vector of components hence the nesting
			std::vector<dynamicRenderMeshComponent>* RenderMeshComponents = static_cast<std::vector<dynamicRenderMeshComponent>*>(rmc[num_vector_of_vectors]);
			std::vector<transformComponent>* TransformComponents = static_cast<std::vector<transformComponent>*>(tc[num_vector_of_vectors]);
			for (size_t i = 0; i < RenderMeshComponents->size();i++) {
				glm::mat4 model = getModelMatrixFromTranform(TransformComponents->at(i));
				scene->getRenderData().updateMat(ctx, RenderMeshComponents->at(i).renderDataLocations, &model);
			}
		}
	}
	void updatePlayer(scene* scene, Entity player, float dt)
	{
		transformComponent* tc = scene->getComponent<transformComponent>(player);
		if (input::getInput().xAxis) {
			tc->position.x += 3 * input::getInput().xAxis * dt;
		}
		if (input::getInput().yAxis) {
			tc->position.y += 3 * input::getInput().yAxis * dt;
		}
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

		std::unordered_map<ComponentEnum, std::vector<void*>> out_components;
		scene->getAllComponents(out_components, ComponentEnum::dynamicBoxColliderComponent | ComponentEnum::rigidBodyComponent | ComponentEnum::transformComponent);
		std::vector<void*> bc = out_components[ComponentEnum::dynamicBoxColliderComponent];
		std::vector<void*> rb = out_components[ComponentEnum::rigidBodyComponent];
		std::vector<void*> tc = out_components[ComponentEnum::transformComponent];
		std::unordered_map<ComponentEnum, std::vector<void*>> out_components_static_box_colliders;
		scene->getAllComponents(out_components_static_box_colliders, ComponentEnum::staticBoxColliderComponent);
		std::vector<void*> bc_static = out_components[ComponentEnum::staticBoxColliderComponent];
		
		for (size_t num_vector_of_vectors = 0; num_vector_of_vectors < bc.size(); num_vector_of_vectors++) { // each archetype has a separate vector of components hence the nesting
			std::vector<dynamicBoxColliderComponent>* RenderMeshComponents = static_cast<std::vector<dynamicBoxColliderComponent>*>(bc[num_vector_of_vectors]);
			std::vector<rigidBodyComponent>* RigidBodyComponents = static_cast<std::vector<rigidBodyComponent>*>(rb[num_vector_of_vectors]);
			std::vector<transformComponent>* TransformComponents = static_cast<std::vector<transformComponent>*>(tc[num_vector_of_vectors]);
			for (size_t i = 0; i < RenderMeshComponents->size(); i++) {
				for (size_t num_vector_of_vectors_static_bc = 0; num_vector_of_vectors_static_bc < bc_static.size(); num_vector_of_vectors_static_bc++) { // each archetype has a separate vector of components hence the nesting
					std::vector<staticBoxColliderComponent>* StaticBoxCollidersComponents = static_cast<std::vector<staticBoxColliderComponent>*>(bc_static[num_vector_of_vectors_static_bc]);
					//for (size_t j = 0; j < StaticBoxCollidersComponents->size(); j++)
						/*if (checkForCollisionAABBAABB(&StaticBoxCollidersComponents->at(i), &StaticBoxCollidersComponents->at(num_vector_of_vectors_static_bc))) {
							collision c;
							c.obj_a = &StaticBoxCollidersComponents->at(i);
							c.obj_b = &StaticBoxCollidersComponents->at(num_vector_of_vectors_static_bc);
							collisions.push_back(c);
						}*/
				
				}
			}
		}
	}


	bool checkForCollisionAABBAABB(dynamicBoxColliderComponent* a, staticBoxColliderComponent* b)
	{
		if (a->min.x > b->max.x || a->max.x < b->min.x) return false;
		if (a->min.y > b->max.y || a->max.y < b->min.y) return false;
		if (a->min.z > b->max.z || a->max.z < b->min.z) return false;
		return true;
	}
}