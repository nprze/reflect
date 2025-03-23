#include "world.h"
#include "components.h"
rfct::world::world() : m_Archetypes(10), m_EntityLocations(0)
{
	m_EntityLocations.reserve(10 * m_Archetypes.size());
}

rfct::world::~world()
{
	Query::cleanUp();
}

void rfct::world::loadWorld(std::string path)
{
	{
		Entity namedEnt = createEntity<NameComponent>({ "entity named" });
		NameComponent nc = getComponent<NameComponent>(namedEnt);
		RFCT_INFO("entity0 name: {}", nc.name);
	}

	{
		Entity e1 = createEntity<NameComponent, DamageComponent>({ "entity 1" }, { 0 });
		NameComponent nc1 = getComponent<NameComponent>(e1);
		RFCT_INFO("entity1 name: {}", nc1.name);
		DamageComponent dc1 = getComponent<DamageComponent>(e1);
		RFCT_INFO("entity1 damage: {}", dc1.damage);
	}

	{
		Entity e2 = createEntity<NameComponent, DamageComponent>({ "entity 2" }, { 100 });
		NameComponent nc2 = getComponent<NameComponent>(e2);
		RFCT_INFO("entity2 name: {}", nc2.name);
		DamageComponent dc2 = getComponent<DamageComponent>(e2);
		RFCT_INFO("entity2 damage: {}", dc2.damage);
	}

	{
		Entity e2 = createEntity<NameComponent, DamageComponent>({ "entity 2" }, { 100 });
		NameComponent nc2 = getComponent<NameComponent>(e2);
		RFCT_INFO("entity name: {}", nc2.name);
		DamageComponent dc2 = getComponent<DamageComponent>(e2);
		RFCT_INFO("entity damage: {}", dc2.damage);

	}
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
		/*if (m_EntityLocations.size() == m_EntityLocations.capacity()) {
			m_EntityLocations.reserve(m_EntityLocations.size() * 2);
		}*/
		EntityID = m_EntityLocations.size();
		m_EntityLocations.emplace_back(EntityLocation(nullptr, 0));
	}
	Archetype<Components...>* arch = Query::getArchetype<Components...>();
	m_EntityLocations[EntityID].archetype = arch; 
	m_EntityLocations[EntityID].locationIndex = arch->addEntity({ EntityID }, std::forward<Components>(components)...);


	return { EntityID };
}

template<typename Component>
Component& rfct::world::getComponent(Entity entity)
{
	EntityLocation& entityLoc = m_EntityLocations.at(entity.id);

	if (!entityLoc.archetype) {
		throw std::runtime_error("Entity does not exist or has no components.");
	}
	BaseArchetype* baseArch = entityLoc.archetype;
	/*
	auto* typedArchetype = dynamic_cast<Archetype<Component>*>(baseArch);

	if (!typedArchetype) {
		throw std::runtime_error("Requested component does not exist for this entity.");
	}*/

	return baseArch->getComponent<Component>(entityLoc.locationIndex);
	
}

