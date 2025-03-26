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
		Entity namedEnt = helloEntity<nameComponent, damageComponent>(nameComponent("named name and damage 0"), damageComponent(0));
		nameComponent* nc = getComponent<nameComponent>(namedEnt);
		RFCT_INFO("entity name: {}", nc->name);
		damageComponent* dc = getComponent<damageComponent>(namedEnt);
		RFCT_INFO("entity damage: {}", dc->damage);
		RFCT_INFO("entity index in archatype: {}", m_EntityLocations[namedEnt].locationIndex);
		RFCT_INFO("current number of archetypes: {}", Query::archetypes.size());
	}
	{
		Entity namedEnt = helloEntity<nameComponent, damageComponent>(nameComponent("named naem and damage 1"), damageComponent(1));
		nameComponent* nc = getComponent<nameComponent>(namedEnt);
		RFCT_INFO("entity name: {}", nc->name);
		damageComponent* dc = getComponent<damageComponent>(namedEnt);
		RFCT_INFO("entity damage: {}", dc->damage);
		RFCT_INFO("entity index in archatype: {}", m_EntityLocations[namedEnt].locationIndex);
	}
	Entity toBeDeletedEntity;
	{
		Entity namedEnt = helloEntity<nameComponent, damageComponent>(nameComponent("named name and damage to be deleted 2"), damageComponent(69));
		nameComponent* nc = getComponent<nameComponent>(namedEnt);
		RFCT_INFO("entity name: {}", nc->name);
		damageComponent* dc = getComponent<damageComponent>(namedEnt);
		RFCT_INFO("entity damage: {}", dc->damage);
		RFCT_INFO("entity index in archatype: {}", m_EntityLocations[namedEnt].locationIndex);
		RFCT_INFO("current number of archetypes: {}", Query::archetypes.size());
		toBeDeletedEntity = namedEnt;
		
	}
	{
		Entity namedEnt = helloEntity<nameComponent, damageComponent>(nameComponent("named name and damage first 3 that is supposed to then become 2"), damageComponent(2));
		nameComponent* nc = getComponent<nameComponent>(namedEnt);
		RFCT_INFO("entity name: {}", nc->name);
		damageComponent* dc = getComponent<damageComponent>(namedEnt);
		RFCT_INFO("entity damage: {}", dc->damage);
		RFCT_INFO("entity index in archatype: {}", m_EntityLocations[namedEnt].locationIndex); 
		goodbyeEntity(toBeDeletedEntity);
		RFCT_INFO("entity index in archatype: {}", m_EntityLocations[namedEnt].locationIndex);
		RFCT_INFO("current number of archetypes: {}", Query::archetypes.size());
	}

	{
		Entity namedEnt = helloEntity<nameComponent, damageComponent>(nameComponent("named name and damage 3"), damageComponent(3));
		nameComponent* nc = getComponent<nameComponent>(namedEnt);
		RFCT_INFO("entity name: {}", nc->name);
		damageComponent* dc = getComponent<damageComponent>(namedEnt);
		RFCT_INFO("entity damage: {}", dc->damage);
		RFCT_INFO("entity index in archatype: {}", m_EntityLocations[namedEnt].locationIndex);
		RFCT_INFO("current number of archetypes: {}", Query::archetypes.size());
	}
	{
		Entity namedEnt = helloEntity<nameComponent>(nameComponent("named just name to be deleted 0"));
		nameComponent* nc = getComponent<nameComponent>(namedEnt);
		RFCT_INFO("entity name: {}", nc->name);
		RFCT_INFO("entity index in archatype: {}", m_EntityLocations[namedEnt].locationIndex);
		goodbyeEntity(namedEnt);
		RFCT_INFO("current number of archetypes: {}", Query::archetypes.size());

	}
	{
		Entity namedEnt = helloEntity<nameComponent>(nameComponent("named just name to have health added"));
		nameComponent* nc = getComponent<nameComponent>(namedEnt);
		RFCT_INFO("entity name: {}", nc->name);
		RFCT_INFO("entity index in archatype: {}", m_EntityLocations[namedEnt].locationIndex);
		RFCT_INFO("current number of archetypes: {}", Query::archetypes.size());
		addComponentToEntity<healthComponent>(namedEnt, healthComponent(4));
		healthComponent* hc = getComponent<healthComponent>(namedEnt);
		RFCT_INFO("entity health: {}", hc->health);
		RFCT_INFO("current number of archetypes: {}", Query::archetypes.size());
		
		//nc = getComponent<nameComponent>(namedEnt);
		//RFCT_INFO("entity name: {}", nc->name);
		//damageComponent* dc = getComponent<damageComponent>(namedEnt);
		//RFCT_INFO("entity damage: {}", dc->damage);
		RFCT_INFO("entity index in archatype: {}", m_EntityLocations[namedEnt].locationIndex);
		RFCT_INFO("current number of archetypes: {}", Query::archetypes.size());

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


template<typename Component>
void rfct::world::addComponentToEntity(Entity entity, Component component)
{
	Archetype* oldArchetype = m_EntityLocations[entity].archetype;
	if ((bool)(oldArchetype->componentsBitmask & Component::EnumValue)) RFCT_CRITICAL("Entity already has component");

	Archetype* newArchetype = Query::getArchetype(m_EntityLocations[entity].archetype->componentsBitmask, Component::EnumValue);

	ComponentEnum components = m_EntityLocations[entity].archetype->componentsBitmask;
	// adding component to new archetype
	while ((bool)components) {
		ComponentEnum selBit = (ComponentEnum)((size_t)components & -(size_t)components);

		switch (selBit) {
			case ComponentEnum::nameComponent: {
				newArchetype->addsingleComponent<nameComponent>(oldArchetype->getComponent<nameComponent>(m_EntityLocations[entity].locationIndex));
				break;
			}
			case ComponentEnum::damageComponent: {
				newArchetype->addsingleComponent<damageComponent>(oldArchetype->getComponent<damageComponent>(m_EntityLocations[entity].locationIndex));
				break;
			}
			case ComponentEnum::healthComponent: {
				newArchetype->addsingleComponent<healthComponent>(oldArchetype->getComponent<healthComponent>(m_EntityLocations[entity].locationIndex));
				break;
			}
			default: {
				RFCT_CRITICAL("Couldn't add entity components");
				break;
			}
		}
		components = static_cast<ComponentEnum>(static_cast<size_t>(components) & (static_cast<size_t>(components) - 1));
	}
	newArchetype->addsingleComponent<Component>(component);

	oldArchetype->removeEntity(m_EntityLocations[entity].locationIndex);
	if (oldArchetype->entities.size() == 0) { 
		Query::removeArchetype(m_EntityLocations[entity].archetype);
	}
	m_EntityLocations[entity].locationIndex = newArchetype->addEntity(entity);
	m_EntityLocations[entity].archetype = newArchetype;
}

void rfct::world::goodbyeEntity(Entity entity)
{
	m_EntityLocations[entity].archetype->removeEntity(m_EntityLocations[entity].locationIndex);
	if (m_EntityLocations[entity].archetype->entities.size() == 0) {
		Query::removeArchetype(m_EntityLocations[entity].archetype);
	}
	m_FreeEntityBlocks.emplace_back(entity);
	m_EntityLocations[entity] = EntityLocation(nullptr, 0);
}

void rfct::world::runEntityTests()
{
	/*
	{
		Entity namedEnt = helloEntity<nameComponent, damageComponent>(nameComponent("named name and damage 0"), damageComponent(0));
		nameComponent* nc = getComponent<nameComponent>(namedEnt);
		RFCT_ASSERT(nc->name == "named name and damage 0"); // entity name
		damageComponent* dc = getComponent<damageComponent>(namedEnt);
		RFCT_ASSERT(dc->damage == 0); // entity damage
		RFCT_ASSERT(m_EntityLocations[namedEnt].locationIndex == 0); // entity index in archetype
		RFCT_ASSERT(Query::archetypes.size() == 2); // number of archetypes
	}

	{
		Entity namedEnt = helloEntity<nameComponent, damageComponent>(nameComponent("named naem and damage 1"), damageComponent(1));
		nameComponent* nc = getComponent<nameComponent>(namedEnt);
		RFCT_ASSERT(nc->name == "named naem and damage 1"); // entity name
		damageComponent* dc = getComponent<damageComponent>(namedEnt);
		RFCT_ASSERT(dc->damage == 1); // entity damage
		RFCT_ASSERT(m_EntityLocations[namedEnt].locationIndex == 1); // entity index in archetype
	}

	Entity toBeDeletedEntity;
	{
		Entity namedEnt = helloEntity<nameComponent, damageComponent>(nameComponent("named name and damage to be deleted 2"), damageComponent(69));
		nameComponent* nc = getComponent<nameComponent>(namedEnt);
		RFCT_ASSERT(nc->name == "named name and damage to be deleted 2"); // entity name
		damageComponent* dc = getComponent<damageComponent>(namedEnt);
		RFCT_ASSERT(dc->damage == 69); // entity damage
		RFCT_ASSERT(m_EntityLocations[namedEnt].locationIndex == 2); // entity index in archetype
		RFCT_ASSERT(Query::archetypes.size() == 2); // number of archetypes
		toBeDeletedEntity = namedEnt;
	}

	{
		Entity namedEnt = helloEntity<nameComponent, damageComponent>(nameComponent("named name and damage first 3 that is supposed to then become 2"), damageComponent(2));
		nameComponent* nc = getComponent<nameComponent>(namedEnt);
		RFCT_ASSERT(nc->name == "named name and damage first 3 that is supposed to then become 2"); // entity name
		damageComponent* dc = getComponent<damageComponent>(namedEnt);
		RFCT_ASSERT(dc->damage == 2); // entity damage
		RFCT_ASSERT(m_EntityLocations[namedEnt].locationIndex == 3); // entity index before deletion
		goodbyeEntity(toBeDeletedEntity);
		RFCT_ASSERT(m_EntityLocations[namedEnt].locationIndex == 2); // entity index after deletion
		RFCT_ASSERT(Query::archetypes.size() == 2); // number of archetypes
	}

	{
		Entity namedEnt = helloEntity<nameComponent, damageComponent>(nameComponent("named name and damage 3"), damageComponent(3));
		nameComponent* nc = getComponent<nameComponent>(namedEnt);
		RFCT_ASSERT(nc->name == "named name and damage 3"); // entity name
		damageComponent* dc = getComponent<damageComponent>(namedEnt);
		RFCT_ASSERT(dc->damage == 3); // entity damage
		RFCT_ASSERT(m_EntityLocations[namedEnt].locationIndex == 3); // entity index
		RFCT_ASSERT(Query::archetypes.size() == 2); // number of archetypes
	}

	{
		Entity namedEnt = helloEntity<nameComponent>(nameComponent("named just name to be deleted 0"));
		nameComponent* nc = getComponent<nameComponent>(namedEnt);
		RFCT_ASSERT(nc->name == "named just name to be deleted 0"); // entity name
		RFCT_ASSERT(m_EntityLocations[namedEnt].locationIndex == 0); // entity index in archetype
		goodbyeEntity(namedEnt);
		RFCT_ASSERT(Query::archetypes.size() == 2); // number of archetypes
	}

	{
		Entity namedEnt = helloEntity<nameComponent>(nameComponent("named just name to have health added"));
		nameComponent* nc = getComponent<nameComponent>(namedEnt);
		RFCT_ASSERT(nc->name == "named just name to have health added"); // entity name
		RFCT_ASSERT(m_EntityLocations[namedEnt].locationIndex == 0); // entity index in archetype
		RFCT_ASSERT(Query::archetypes.size() == 2); // number of archetypes

		addComponentToEntity<healthComponent>(namedEnt, healthComponent(4));
		healthComponent* hc = getComponent<healthComponent>(namedEnt);
		RFCT_ASSERT(hc->health == 4); // entity health
		RFCT_ASSERT(Query::archetypes.size() == 2); // number of archetypes remains 2 after adding healthComponent
		RFCT_ASSERT(m_EntityLocations[namedEnt].locationIndex == 0); // entity index remains valid
	}
	*/
}
