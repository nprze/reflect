#pragma once
#include "render_data.h"

namespace rfct {
	class world;
	struct frameContext;
	class scene {
	public:
		entity camera;
		scene(world* worldArg);
		~scene();
		void onUpdate(frameContext* context);
		void loadScene(std::string path);
		inline void unloadScene() {};
		inline sceneRenderData& getRenderData() { return m_RenderData; };

		entity createStaticRenderingEntity(std::vector<Vertex>* vertices, glm::mat4* model);
		entity createDynamicRenderingEntity(std::vector<Vertex>* vertices, glm::mat4* model);
		void updateTransformData(frameContext* ctx, entity entityToUpdate);


		world* getWorld() { return m_World; }

		entity sceneEntity; // root of all object in this scene
	private:
		sceneRenderData m_RenderData;
		world* m_World;

		entity epicRotatingTriangle;
	};
}