#include "scene.h"
#include "components.h"
#include <glm/glm.hpp>
#include "renderer_p\renderer.h"
#include "components_util.h"
#include "archetype.h"
#include "transform.h"

rfct::scene::scene(world* worldArg) : m_World(worldArg), m_EntityLocations(0)
{
	m_EntityLocations.reserve(100);
	m_Query.archetypes.reserve(5);
}

rfct::scene::~scene()
{
	m_Query.cleanUp();
}


void rfct::scene::onUpdate(frameContext* context)
{
	// Update systems
	//static float deltaTime = 0.f;
	//deltaTime += context->dt;
	transformComponent* tc = world::getWorld().getCurrentScene().getComponent<transformComponent>(epicRotatingTriangle);
	//if (deltaTime > 1.f) {
		//deltaTime = 0.f;

		if (input::getInput().xAxis) {
			tc->position.x += 3 * input::getInput().xAxis * context->dt;
		}
		if (input::getInput().yAxis) {
			tc->position.y += 3 * input::getInput().yAxis * context->dt;
		}
	//}
	updateTransformData(context, epicRotatingTriangle);
	getComponent<cameraComponent>(camera); 
	cameraComponentOnUpdate(context->dt, *tc);

}


void rfct::scene::loadScene(std::string path)
{
	runEntityTests();
	m_Query.cleanUp();
	camera = helloEntity<cameraComponent>(cameraComponent{ glm::vec3(0.f, 0.f, 2.0f), glm::vec3(0), 45.f, renderer::getRen().getAspectRatio(), 0.f, 100.f });
	m_RenderData.startTransferStatic();
	{

		std::vector<Vertex> vertices = {
			{{0.0f, -0.5f, 0.f}, {1.0f, 0.0f, 0.0f},0,0},
			{{0.5f, 0.5f, 0.f}, {0.0f, 1.0f, 0.0f},0,0},
			{{-0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f},0,0}
		};
		transformComponent tc = {};
		createStaticRenderingEntity(&vertices, &tc);
	}
	{

		std::vector<Vertex> vertices = {
			{{0.0f, -0.5f, 0.f}, {0.0f, 0.0f, 0.0f},0,0},
			{{0.5f, 0.5f, 0.f}, {0.0f, 1.0f, 0.0f},0,0},
			{{-0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f},0,0}
		};
		transformComponent tc = {};
		tc.position.x -= 1.f;
		createStaticRenderingEntity(&vertices, &tc);
	}
	m_RenderData.endTransferStatic();
	std::vector<Vertex> vertices = {
		{{0.0f, -0.5f, 0.f}, {1.0f, 1.0f, 1.0f},0,0},
		{{0.5f, 0.5f, 0.f}, {0.0f, 1.0f, 0.0f},0,0},
		{{-0.5f, 0.5f, 0.f}, {0.0f, 0.0f, 1.0f},0,0}
	};
	transformComponent tc = {};
	tc.position.x += 1.f;
	epicRotatingTriangle = createDynamicRenderingEntity(&vertices, &tc);
}

template<typename... Components>
rfct::Entity rfct::scene::helloEntity(Components&&... componentMap)
{
	RFCT_PROFILE_FUNCTION();
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
	Archetype* arch = m_Query.getArchetype<Components...>();
	
	m_EntityLocations[EntityID].archetype = arch; 
	m_EntityLocations[EntityID].locationIndex = arch->addEntity<Components...>({ EntityID }, std::forward<Components>(componentMap)...);
	
	
	return { EntityID };
}


template<typename Component>
Component* rfct::scene::getComponent(Entity entity)
{
	RFCT_PROFILE_FUNCTION();
	if (!((bool)(m_EntityLocations[entity.id].archetype->componentsBitmask & Component::EnumValue))) RFCT_CRITICAL("Entity does not have component");
	return &m_EntityLocations[entity.id].archetype->getComponent<Component>(m_EntityLocations[entity.id].locationIndex);
}


template<typename Component>
void rfct::scene::addComponentToEntity(Entity entity, Component component)
{
	RFCT_PROFILE_FUNCTION();
	Archetype* oldArchetype = m_EntityLocations[entity].archetype;
	if ((bool)(oldArchetype->componentsBitmask & Component::EnumValue)) RFCT_CRITICAL("Entity already has component");

	Archetype* newArchetype = m_Query.getArchetype(m_EntityLocations[entity].archetype->componentsBitmask, Component::EnumValue);

	ComponentEnum components = m_EntityLocations[entity].archetype->componentsBitmask;
	// adding component to new archetype
	while ((bool)components) {
		ComponentEnum selBit = (ComponentEnum)((size_t)components & -(size_t)components);

		RFCT_ADD_SINGLE_COMPONENT(selBit, newArchetype, oldArchetype, entity);

		components = static_cast<ComponentEnum>(static_cast<size_t>(components) & (static_cast<size_t>(components) - 1));
	}
	newArchetype->addsingleComponent<Component>(component);

	oldArchetype->removeEntity(m_EntityLocations[entity].locationIndex);
	if (oldArchetype->entities.size() == 0) { 
		m_Query.removeArchetype(m_EntityLocations[entity].archetype);
	}
	m_EntityLocations[entity].locationIndex = newArchetype->addEntity(entity);
	m_EntityLocations[entity].archetype = newArchetype;
}

void rfct::scene::getAllComponents(std::unordered_map<ComponentEnum, std::vector<void*>>& out_components, ComponentEnum requestedComponents)
{
	RFCT_PROFILE_FUNCTION();
	std::vector<Archetype*> archetypes;
	archetypes.reserve(5);
	m_Query.getAllArchetypesWithComponents(requestedComponents, archetypes);

	// adding component to new archetype
	while ((bool)requestedComponents) {
		ComponentEnum selBit = (ComponentEnum)((size_t)requestedComponents & -(size_t)requestedComponents);
		std::vector<void*>& components = out_components[selBit];
		components.reserve(5);
		for (Archetype* arch : archetypes) {
			components.emplace_back(arch->componentMap[selBit]);
		}

		requestedComponents = static_cast<ComponentEnum>(static_cast<size_t>(requestedComponents) & (static_cast<size_t>(requestedComponents) - 1));
	}
}

rfct::Entity rfct::scene::createStaticRenderingEntity(std::vector<Vertex>* vertices, transformComponent* tranform)
{
	glm::mat4 model = getModelMatrixFromTranform(*tranform);
	staticRenderMeshComponent rmc = { m_RenderData.addStaticObject(vertices, &model) };
	return helloEntity<transformComponent, staticRenderMeshComponent>(std::move(*tranform), std::move(rmc));
}

rfct::Entity rfct::scene::createDynamicRenderingEntity(std::vector<Vertex>* vertices, transformComponent* tranform)
{
	glm::mat4 model = getModelMatrixFromTranform(*tranform);
	dynamicRenderMeshComponent rmc = { m_RenderData.addDynamicObject(vertices, &model) };
	return helloEntity<transformComponent, dynamicRenderMeshComponent>(std::move(*tranform), std::move(rmc));
}

void rfct::scene::updateTransformData(frameContext* ctx, Entity ent)
{
	transformComponent* tc = getComponent<transformComponent>(ent);
	dynamicRenderMeshComponent* rmc = getComponent<dynamicRenderMeshComponent>(ent);
	glm::mat4 model = getModelMatrixFromTranform(*tc);
	m_RenderData.updateMat(ctx, rmc->renderDataLocations, &model);
}

void rfct::scene::goodbyeEntity(Entity entity)
{
	RFCT_PROFILE_FUNCTION();
	m_EntityLocations[entity].archetype->removeEntity(m_EntityLocations[entity].locationIndex);
	if (m_EntityLocations[entity].archetype->entities.size() == 0) {
		m_Query.removeArchetype(m_EntityLocations[entity].archetype);
	}
	m_FreeEntityBlocks.emplace_back(entity);
	m_EntityLocations[entity] = EntityLocation(nullptr, 0);
}

void rfct::scene::runEntityTests()
	{
		{
			Entity namedEnt = helloEntity<nameComponent, damageComponent>(nameComponent("named name and damage 0"), damageComponent(0));
			nameComponent* nc = getComponent<nameComponent>(namedEnt);
			RFCT_ASSERT((nc->name == std::string("named name and damage 0"))); // entity name
			damageComponent* dc = getComponent<damageComponent>(namedEnt);
			RFCT_ASSERT(dc->damage == 0); // entity damage
			RFCT_ASSERT(m_EntityLocations[namedEnt].locationIndex == 0); // entity index in archetype
			RFCT_ASSERT((m_Query.archetypes.size() == 1)); // number of archetypes
		}

		{
			Entity namedEnt = helloEntity<nameComponent, damageComponent>(nameComponent("named naem and damage 1"), damageComponent(1));
			nameComponent* nc = getComponent<nameComponent>(namedEnt);
			RFCT_ASSERT((nc->name == std::string("named naem and damage 1"))); // entity name
			damageComponent* dc = getComponent<damageComponent>(namedEnt);
			RFCT_ASSERT(dc->damage == 1); // entity damage
			RFCT_ASSERT(m_EntityLocations[namedEnt].locationIndex == 1); // entity index in archetype
		}

		Entity toBeDeletedEntity;
		{
			Entity namedEnt = helloEntity<nameComponent, damageComponent>(nameComponent("named name and damage to be deleted 2"), damageComponent(69));
			nameComponent* nc = getComponent<nameComponent>(namedEnt);
			RFCT_ASSERT((nc->name == std::string("named name and damage to be deleted 2"))); // entity name
			damageComponent* dc = getComponent<damageComponent>(namedEnt);
			RFCT_ASSERT((bool)(dc->damage == 69)); // entity damage
			RFCT_ASSERT((bool)(m_EntityLocations[namedEnt].locationIndex == 2)); // entity index in archetype
			RFCT_ASSERT((bool)(m_Query.archetypes.size() == 1)); // number of archetypes
			toBeDeletedEntity = namedEnt;
		}

		{
			Entity namedEnt = helloEntity<nameComponent, damageComponent>(nameComponent("named name and damage first 3 that is supposed to then become 2"), damageComponent(2));
			nameComponent* nc = getComponent<nameComponent>(namedEnt);
			RFCT_ASSERT((nc->name == std::string("named name and damage first 3 that is supposed to then become 2"))); // entity name
			damageComponent* dc = getComponent<damageComponent>(namedEnt);
			RFCT_ASSERT((bool)(dc->damage == 2)); // entity damage
			RFCT_ASSERT((bool)(m_EntityLocations[namedEnt].locationIndex == 3)); // entity index before deletion
			goodbyeEntity(toBeDeletedEntity);
			RFCT_ASSERT(m_EntityLocations[namedEnt].locationIndex == 2); // entity index after deletion
			RFCT_ASSERT(m_Query.archetypes.size() == 1); // number of archetypes
		}

		{
			Entity namedEnt = helloEntity<nameComponent, damageComponent>(nameComponent("named name and damage 3"), damageComponent(3));
			nameComponent* nc = getComponent<nameComponent>(namedEnt);
			RFCT_ASSERT((nc->name == std::string("named name and damage 3"))); // entity name
			damageComponent* dc = getComponent<damageComponent>(namedEnt);
			RFCT_ASSERT(dc->damage == 3); // entity damage
			RFCT_ASSERT(m_EntityLocations[namedEnt].locationIndex == 3); // entity index
			RFCT_ASSERT(m_Query.archetypes.size() == 1); // number of archetypes
		}

		{
			Entity namedEnt = helloEntity<nameComponent>(nameComponent("named just name to be deleted 0"));
			nameComponent* nc = getComponent<nameComponent>(namedEnt);
			RFCT_ASSERT((nc->name == std::string("named just name to be deleted 0"))); // entity name
			RFCT_ASSERT(m_EntityLocations[namedEnt].locationIndex == 0); // entity index in archetype
			goodbyeEntity(namedEnt);
			RFCT_ASSERT(m_Query.archetypes.size() == 1); // number of archetypes
		}

		{
			Entity namedEnt = helloEntity<nameComponent>(nameComponent("named just name to have health added"));
			nameComponent* nc = getComponent<nameComponent>(namedEnt);
			RFCT_ASSERT((nc->name == std::string("named just name to have health added"))); // entity name
			RFCT_ASSERT(m_EntityLocations[namedEnt].locationIndex == 0); // entity index in archetype
			RFCT_ASSERT(m_Query.archetypes.size() == 2); // number of archetypes


			addComponentToEntity<healthComponent>(namedEnt, healthComponent(4));
			healthComponent* hc = getComponent<healthComponent>(namedEnt);
			nc = getComponent<nameComponent>(namedEnt);
			RFCT_ASSERT((nc->name == std::string("named just name to have health added")));
			RFCT_ASSERT(hc->health == 4); // entity health
			RFCT_ASSERT(m_Query.archetypes.size() == 2); // number of archetypes remains 2 after adding healthComponent
			RFCT_ASSERT(m_EntityLocations[namedEnt].locationIndex == 0); // entity index remains valid
		}

		{
			Entity namedEnt = helloEntity<nameComponent>(nameComponent("named just name to have damage added"));
			nameComponent* nc = getComponent<nameComponent>(namedEnt);
			RFCT_ASSERT((nc->name == std::string("named just name to have damage added"))); // entity name
			RFCT_ASSERT(m_EntityLocations[namedEnt].locationIndex == 0); // entity index in archetype
			RFCT_ASSERT(m_Query.archetypes.size() == 3); // number of archetypes


			addComponentToEntity<damageComponent>(namedEnt, damageComponent(4));
			damageComponent* hc = getComponent<damageComponent>(namedEnt);
			RFCT_ASSERT(hc->damage == 4); // entity health
			RFCT_ASSERT(m_Query.archetypes.size() == 2); // number of archetypes remains 2 after adding healthComponent
			RFCT_ASSERT(m_EntityLocations[namedEnt].locationIndex == 4); // entity index remains valid
		}
		{
			std::unordered_map<ComponentEnum, std::vector<void*>> out_components;
			getAllComponents(out_components, ComponentEnum::nameComponent);
			std::vector<void*> nameComponents = out_components[ComponentEnum::nameComponent];
			size_t componentCount = 0;
			for (void* ncs : nameComponents) {
				std::vector<nameComponent>* nc = static_cast<std::vector<nameComponent>*>(ncs);
				for (nameComponent n : *nc) {
					componentCount++;
				}
			}
			RFCT_ASSERT(componentCount == 6);
		}
		{
			std::unordered_map<ComponentEnum, std::vector<void*>> out_components;
			ComponentEnum comps= ComponentEnum::nameComponent | ComponentEnum::healthComponent;
			getAllComponents(out_components,comps);
			std::vector<void*> nameComponents = out_components[ComponentEnum::nameComponent];
			size_t componentCount = 0;
			for (void* ncs : nameComponents) {
				std::vector<nameComponent>* nc = static_cast<std::vector<nameComponent>*>(ncs);
				for (nameComponent n : *nc) {
					componentCount++;
				}
			}
			RFCT_ASSERT(componentCount == 1);
			componentCount = 0;
			std::vector<void*> damageComponent_var = out_components[ComponentEnum::healthComponent];
			for (void* dcs : damageComponent_var) {
				std::vector<healthComponent>* dc = static_cast<std::vector<healthComponent>*>(dcs);
				for (healthComponent d : *dc) {
					componentCount++;
				}
			}
			RFCT_ASSERT(componentCount == 1);
		}
	}
