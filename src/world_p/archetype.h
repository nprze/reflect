#pragma once
#include <vector>
#include <tuple>
#include <stdexcept>
#include <any>
#include "components.h"
#include "components_util.h"

namespace rfct {
    struct Query;

        class Archetype {

        public:
            ComponentEnum componentsBitmask;
            std::vector<Entity> entities;
		    std::unordered_map<ComponentEnum, void*> componentMap; // void* is a pointer to std::vector that holds the actual components
            std::vector<std::function<void()>> cleanupFunctions;


            inline Archetype(ComponentEnum components)
                : componentsBitmask(components) {
                entities.reserve(RFCT_DEFAULT_ARCHETYPE_COMPONENTS_COUNT);
            }

            inline ~Archetype() {

                entities.clear();
                for (auto& cleanup : cleanupFunctions) {
                    cleanup();
                }
                componentMap.clear();
            }

		    template<typename... Components>
            inline size_t addEntity(Entity entity, Components&&... componentValues) {
                entities.emplace_back(entity);

                ((
                    [&] {
                            auto* componentVector = static_cast<std::vector<std::decay_t<Components>>*>(componentMap[Components::EnumValue]);
                            Components comp = std::forward<Components>(componentValues);
                            componentVector->emplace_back(comp);
                    }()
                        ), ...);

                return entities.size() - 1;
            }

            template<typename Component>
            inline void addsingleComponent(Component componentValue) {
                auto* componentVector = static_cast<std::vector<Component>*>(componentMap[Component::EnumValue]);
                componentVector->emplace_back(componentValue);
            }
            
            void removeEntity(size_t index);

            template <typename Component>
            inline Component& getComponent(size_t index) {
                return ((static_cast<std::vector<Component>*>(componentMap[Component::EnumValue])))->at(index);
            }

            template <typename Component>
            inline std::vector<Component>& getComponents(ComponentEnum componentEnum) {
                return *(static_cast<std::vector<Component>*>(componentMap[Component::EnumValue]));
            }
        

        private:
		    template <typename ComponentType>
            inline void addComponent() {
				if (!(bool)(componentsBitmask & ComponentType::EnumValue)) RFCT_CRITICAL("attempting to add component to archetype which wasn't supposed to have that component");
                if (componentMap.find(ComponentType::EnumValue) != componentMap.end())return;
			    auto* vec = new std::vector<ComponentType>();
                componentMap[ComponentType::EnumValue] = vec;
                cleanupFunctions.push_back([vec]() { delete vec; });
                (static_cast<std::vector<ComponentType>*>(componentMap[ComponentType::EnumValue]))->reserve(RFCT_DEFAULT_ARCHETYPE_COMPONENTS_COUNT);
            }
		    friend struct Query;
        };

        struct Query {
            std::vector<Archetype*> archetypes;

            inline void cleanUp() {
                for (Archetype* archetype : archetypes) {
                    delete archetype;
                }
                archetypes.clear();
            }

            template<typename... Components>
            inline Archetype* getArchetype() {
                ComponentEnum requestedComponents = (Components::EnumValue | ...);
			    for (Archetype* archetype : archetypes) {
				    if (archetype->componentsBitmask == requestedComponents) {
					    return archetype;
				    }
			    }
			    addArchetype<Components...>();
			    return archetypes.back();
            }


            inline Archetype* getArchetype(ComponentEnum components, ComponentEnum newComponent = ComponentEnum::None ) {
                for (Archetype* archetype : archetypes) {
                    if (archetype->componentsBitmask == (components | newComponent)) {
                        return archetype;
                    }
                }
                components |= newComponent;
                Archetype* archetype = new Archetype(components);
                archetypes.push_back(archetype);

                while ((bool)components) {
                    ComponentEnum selBit = (ComponentEnum)((size_t)components & -(size_t)components);

					RFCT_ADD_COMPONENT(selBit, archetype);
                    
                    components = static_cast<ComponentEnum>(static_cast<size_t>(components) & (static_cast<size_t>(components) - 1));

                };                
                return archetypes.back();
            }
		    template<typename... Components>
            inline void addArchetype() {
                ComponentEnum components = (Components::EnumValue | ...);
			    Archetype* archetype = new Archetype(components);
			    archetypes.push_back(archetype);
                (archetype->addComponent<Components>(), ...);
            }
            inline void removeArchetype(Archetype* archetypeToBeRemoved) {
                RFCT_ASSERT(archetypeToBeRemoved->entities.size() == 0); // attempting to remove archetype with entities still in it
                archetypes.erase(std::remove(archetypes.begin(), archetypes.end(), archetypeToBeRemoved), archetypes.end());
                delete archetypeToBeRemoved;
            }
			inline void getAllArchetypesWithComponents(ComponentEnum requestedComponents, std::vector<Archetype*>& out_archetypes) {
				for (Archetype* archetype : archetypes) {
					if ((bool)((archetype->componentsBitmask & requestedComponents) == requestedComponents)) {
						out_archetypes.emplace_back(archetype);
					}
				}
			}
        };
    }
