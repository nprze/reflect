#include "world.h"
#include "components.h"
rfct::world::world() : m_EntityLocations(0)
{
	m_EntityLocations.reserve(100);
}

rfct::world rfct::world::currentWorld;
rfct::world::~world()
{
	Query::cleanUp();
}

void rfct::world::onUpdate(float dt)
{
	{
		Entity e0 = { 0 };
		nameComponent* nc = getComponent<nameComponent>(e0);
	}
	/* {
		Entity e0 = { 1 };
		healthComponent* nc = getComponent<healthComponent>(e0);
		if (nc) {
			RFCT_INFO("entity0 health: {}", nc->health);
		}
		else {
			RFCT_INFO("entity0 has no health component");
		}
	}*/
}

void rfct::world::loadWorld(std::string path)
{
	{
		Entity namedEnt = helloEntity<nameComponent, damageComponent>(nameComponent("named0"), damageComponent(10));
		nameComponent* nc = getComponent<nameComponent>(namedEnt);
		RFCT_INFO("entity name: {}", nc->name);
		damageComponent* dc = getComponent<damageComponent>(namedEnt);
		RFCT_INFO("entity damage: {}", dc->damage);
		RFCT_INFO("entity index in archatype: {}", m_EntityLocations[namedEnt].locationIndex);
		//goodbyeEntity(namedEnt);
	}
	{
		Entity namedEnt = helloEntity<nameComponent, damageComponent>(nameComponent("named1"), damageComponent(20));
		nameComponent* nc = getComponent<nameComponent>(namedEnt);
		RFCT_INFO("entity name: {}", nc->name);
		damageComponent* dc = getComponent<damageComponent>(namedEnt);
		RFCT_INFO("entity damage: {}", dc->damage);
		RFCT_INFO("entity index in archatype: {}", m_EntityLocations[namedEnt].locationIndex);
	}
	{
		Entity namedEnt = helloEntity<nameComponent, damageComponent>(nameComponent("named to be deleted"), damageComponent(20));
		nameComponent* nc = getComponent<nameComponent>(namedEnt);
		RFCT_INFO("entity name: {}", nc->name);
		damageComponent* dc = getComponent<damageComponent>(namedEnt);
		RFCT_INFO("entity damage: {}", dc->damage);
		RFCT_INFO("entity index in archatype: {}", m_EntityLocations[namedEnt].locationIndex);
		goodbyeEntity(namedEnt);
	}
	{
		Entity namedEnt = helloEntity<nameComponent, damageComponent>(nameComponent("named2"), damageComponent(20));
		nameComponent* nc = getComponent<nameComponent>(namedEnt);
		RFCT_INFO("entity name: {}", nc->name);
		damageComponent* dc = getComponent<damageComponent>(namedEnt);
		RFCT_INFO("entity damage: {}", dc->damage);
		RFCT_INFO("entity index in archatype: {}", m_EntityLocations[namedEnt].locationIndex);
	}
	{
		Entity namedEnt = helloEntity<nameComponent>(nameComponent("named3"));
		nameComponent* nc = getComponent<nameComponent>(namedEnt);
		RFCT_INFO("entity name: {}", nc->name);
		RFCT_INFO("entity index in archatype: {}", m_EntityLocations[namedEnt].locationIndex);

	}
}

template<typename... Components>
rfct::Entity rfct::world::helloEntity(Components&&... componentMap)
{

	ComponentEnum components = (Components::EnumValue | ...);
  	size_t EntityID = 0;
	if (m_FreeEntityBlocks.size() != 0) { 
		EntityID = m_FreeEntityBlocks.back(); 
		m_FreeEntityBlocks.pop_back();
		m_EntityLocations[EntityID] = EntityLocation(nullptr, 0);
	}
	else
	{
		EntityID = m_EntityLocations.size();
		m_EntityLocations.emplace_back(EntityLocation(nullptr, 0));
	}
	Archetype* arch = Query::getArchetype<Components...>();
	
	m_EntityLocations[EntityID].archetype = arch; 
	m_EntityLocations[EntityID].locationIndex = arch->addEntity<Components...>({ EntityID }, std::forward<Components>(componentMap)...);
	
	
	return { EntityID };
}


template<typename Component>
Component* rfct::world::getComponent(Entity entity)
{
	if (!((bool)(m_EntityLocations[entity.id].archetype->componentsBitmask & Component::EnumValue))) RFCT_CRITICAL("Entity does not have component");
	return &m_EntityLocations[entity.id].archetype->getComponent<Component>(m_EntityLocations[entity.id].locationIndex);
}

void rfct::world::goodbyeEntity(Entity entity)
{
	m_EntityLocations[entity].archetype->removeEntity(m_EntityLocations[entity].locationIndex);
	m_FreeEntityBlocks.emplace_back(entity);
	m_EntityLocations[entity] = EntityLocation(nullptr, 0);
}