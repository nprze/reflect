#pragma once
#include "render_data.h"
#include "archetype.h"
namespace rfct {
	struct EntityLocation {
		Archetype* archetype = nullptr;
		size_t locationIndex = 0;
	};
	class world;
	struct frameContext;
	class scene {
	public:
		Entity camera;
		scene(world* worldArg);
		~scene();
		void onUpdate(frameContext* context);
		void loadScene(std::string path);
		inline void unloadScene() {};
		inline sceneRenderData& getRenderData() { return m_RenderData; };


		// ecs
		template<typename... Components>
		Entity helloEntity(Components&&... componentMap);

		template<typename Component>
		Component* getComponent(Entity entity);

		template<typename Component>
		void addComponentToEntity(Entity entity, Component component);

		void goodbyeEntity(Entity entity);
		void runEntityTests();

		void getAllComponents(std::unordered_map<ComponentEnum, std::vector<void*>>& out_components, ComponentEnum requestedComponents);

		// add entities
		Entity createStaticRenderingEntity(std::vector<Vertex>* vertices, transformComponent* tranform);
		Entity createDynamicRenderingEntity(std::vector<Vertex>* vertices, transformComponent* tranform);

		void updateTransformData(frameContext* ctx, Entity ent);

		world* getWorld() { return m_World; }

	private:
		std::vector<EntityLocation> m_EntityLocations; // Entity is an index by which we adress this to get the actual location of components
		std::vector<size_t> m_FreeEntityBlocks; // holds indices of m_EntityLocations of components which have been deleted and are waiting to be reused

		sceneRenderData m_RenderData;
		world* m_World;
		Query m_Query;

		rfct::Entity epicRotatingTriangle;

		friend class Archetype;
	};
}