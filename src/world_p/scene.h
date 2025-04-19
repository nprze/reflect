#pragma once
#include "render_data.h"

namespace rfct {
	class world;
	struct frameContext;
	class scene {
	public:
		scene(world* worldArg);
		~scene();
		void onUpdate(frameContext* context);
		void loadScene(std::string path);
		inline void unloadScene() {};
		inline sceneRenderData& getRenderData() { return m_RenderData; };

		entity createStaticMesh(std::string path, glm::vec2 size, glm::vec2 pos);
		entity createStaticRect(staticBoxColliderComponent* bounds, glm::vec3 color = glm::vec3(1.f, 1.f, 1.f));
		entity createStaticRenderingEntity(std::vector<Vertex>* vertices, glm::mat4* model);
		
		entity createDynamicRect(dynamicBoxColliderComponent* bounds, glm::vec3 color = glm::vec3(1.f, 1.f, 1.f));
		entity createDynamicRenderingEntity(std::vector<Vertex>* vertices, glm::mat4* model);
		void updateTransformData(frameContext* ctx, entity entityToUpdate);


		world* getWorld() { return m_World; }

		entity camera;
		entity sceneEntity; // root of all objects in this scene
	private:
		sceneRenderData m_RenderData;
		world* m_World;

		entity epicRotatingTriangle;
	};
};