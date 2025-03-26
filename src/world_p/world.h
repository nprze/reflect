#pragma once
#include "archetype.h"
namespace rfct {
	struct EntityLocation {
		Archetype* archetype = nullptr;
		size_t locationIndex = 0;
	};
	class world {
		static world currentWorld;
	public:
		static world& getWorld() { return currentWorld; }
		world();
		~world();
		void onUpdate(float dt);
		void loadWorld(std::string path);
		template<typename... Components>
		Entity helloEntity(Components&&... componentMap);

		template<typename Component>
		Component* getComponent(Entity entity);

		template<typename Component>
		void addComponentToEntity(Entity entity, Component component);

		void goodbyeEntity(Entity entity);
		void runEntityTests();

	private:
		std::vector<EntityLocation> m_EntityLocations; // Entity is an index by which we adress this to get the actual location of components
		std::vector<size_t> m_FreeEntityBlocks; // holds indices of m_EntityLocations of components which have been deleted and are waiting to be reused
		friend class Archetype;
	};
}