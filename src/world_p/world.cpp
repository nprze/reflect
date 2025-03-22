#include "world.h"
#include "components.h"
rfct::world::world() : m_Archetypes(10)
{

}

rfct::world::~world()
{
	Query::cleanUp();
}

void rfct::world::loadWorld(std::string path)
{
	Entity namedEnt = createEntity<NameComponent>({"name"});

}

template<typename... Components>
rfct::Entity rfct::world::createEntity(Components&&... components)
{
	size_t EntityID = 0;
	if (m_FreeEntityBlocks.size() != 0) { 
		EntityID = m_FreeEntityBlocks.back(); 
		m_FreeEntityBlocks.pop_back();
		m_EntityLocations[EntityID] = EntityLocation(nullptr, 0);
	}
	else
	{
		if (m_EntityLocations.size() == m_EntityLocations.capacity()) {
			m_EntityLocations.reserve(m_EntityLocations.size() * 2);
		}
		EntityID = m_EntityLocations.size();
		m_EntityLocations.emplace_back(EntityLocation(nullptr, 0));
	}
	Archetype<Components...>* arch = Query::getArchetype<Components...>();
	m_EntityLocations[EntityID].archetype = arch; 
	arch->addEntity({ EntityID }, std::forward<Components>(components)...);


	return { EntityID };
}

