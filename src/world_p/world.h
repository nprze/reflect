#pragma once
#include "archetype.h"
namespace rfct {
	struct EntityLocation {
		BaseArchetype* archetype = nullptr;
		size_t locationIndex = 0;
	};
	class world {
	public:
		world();
		~world();
		void loadWorld(std::string path);
		template<typename... Components>
		Entity createEntity(Components&&... components);

		template<typename Components>
		Components& getComponent(Entity entity);

	private:
		std::vector<unique<BaseArchetype>> m_Archetypes;
		std::vector<EntityLocation> m_EntityLocations; // Entity is an index by which we adress this to get the actual location of components
		std::vector<size_t> m_FreeEntityBlocks; // holds indices of m_EntityLocations of components which have been deleted and are waiting to be reused
	};
}