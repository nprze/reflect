#include "world.h"
#include "components.h"
rfct::world::world() : m_EntityLocations(0)
{
	m_EntityLocations.reserve(100);
}

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
		nameComponent nameComp{};
		nameComp.name = "entity named";
		Entity namedEnt = helloEntity<nameComponent>(ComponentEnum::nameComponent, std::move(nameComp));

		/*
		NameComponent* nc = getComponent<NameComponent>(namedEnt);
		RFCT_INFO("entity name: {}", nc->name);
		goodbyeEntity(namedEnt);*/
	}

	/*
	{
		Entity e1 = helloEntity<NameComponent, DamageComponent>(ComponentEnum::None, { "entity 1" }, { 0 });
		NameComponent* nc1 = getComponent<NameComponent>(e1);
		RFCT_INFO("entity name: {}", nc1->name);
		DamageComponent* dc1 = getComponent<DamageComponent>(e1);
		RFCT_INFO("entity damage: {}", dc1->damage);
	}

	{
		Entity e1 = helloEntity<NameComponent>(ComponentEnum::None, { "entity with name" });
		NameComponent* nc1 = getComponent<NameComponent>(e1);
		RFCT_INFO("entity name: {}", nc1->name);
	}

	{
		Entity e1 = helloEntity<NameComponent, DamageComponent>(ComponentEnum::None, { "entity 1" }, { 100 });
		NameComponent* nc1 = getComponent<NameComponent>(e1);
		RFCT_INFO("entity name: {}", nc1->name);
		DamageComponent* dc1 = getComponent<DamageComponent>(e1);
		RFCT_INFO("entity damage: {}", dc1->damage);
		goodbyeEntity(e1);
	}

	{
		Entity e1 = helloEntity<NameComponent, DamageComponent>(ComponentEnum::None, { "entity yeepie" }, { 0 });
		NameComponent* nc1 = getComponent<NameComponent>(e1);
		RFCT_INFO("entity name: {}", nc1->name);
		DamageComponent* dc1 = getComponent<DamageComponent>(e1);
		RFCT_INFO("entity damage: {}", dc1->damage);
	}*/
}

template<typename... Components>
rfct::Entity rfct::world::helloEntity(ComponentEnum components,Components&&... componentMap)
{
	
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
	Archetype* arch = Query::getArchetype<Components...>(components);
	
	m_EntityLocations[EntityID].archetype = arch; 
	m_EntityLocations[EntityID].locationIndex = arch->addEntity<Components...>({ EntityID }, std::forward<Components>(componentMap)...);
	
	
	return { EntityID };
}


template<typename Component>
Component* rfct::world::getComponent(Entity entity)
{
	/*
	EntityLocation& entityLoc = m_EntityLocations.at(entity.id);

	if (!entityLoc.archetype) {
		RFCT_CRITICAL("Entity does not exist or has no components.");
	}
	BaseArchetype* baseArch = entityLoc.archetype;

	return baseArch->getComponent<Component>(entityLoc.locationIndex);
	*/
	return nullptr;
}

void rfct::world::goodbyeEntity(Entity entity)
{
	/*
	m_FreeEntityBlocks.emplace_back(entity);
	m_EntityLocations[entity].archetype->removeEntity(m_EntityLocations[entity].locationIndex);
	m_EntityLocations[entity] = EntityLocation(nullptr, 0);*/
}