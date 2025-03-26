#pragma once
#include <vector>
#include <tuple>
#include <stdexcept>
#include <any>
#include "components.h"

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
				if (!(bool)(componentsBitmask & ComponentType::EnumValue)) return;
                if (componentMap.find(ComponentType::EnumValue) != componentMap.end())return;
			    auto* vec = new std::vector<ComponentType>();
                componentMap[ComponentType::EnumValue] = vec;
                cleanupFunctions.push_back([vec]() { delete vec; });
                (static_cast<std::vector<ComponentType>*>(componentMap[ComponentType::EnumValue]))->reserve(RFCT_DEFAULT_ARCHETYPE_COMPONENTS_COUNT);
            }
		    friend struct Query;
        };

        struct Query {
            static std::vector<Archetype*> archetypes;

            inline static void cleanUp() {
                for (Archetype* archetype : archetypes) {
                    delete archetype;
                }
                archetypes.clear();
            }

            template<typename... Components>
            inline static Archetype* getArchetype() {
                ComponentEnum requestedComponents = (Components::EnumValue | ...);
			    for (Archetype* archetype : archetypes) {
				    if (archetype->componentsBitmask == requestedComponents) {
					    return archetype;
				    }
			    }
			    addArchetype<Components...>();
			    return archetypes.back();
            }
		    template<typename... Components>
            inline static void addArchetype() {
                ComponentEnum components = (Components::EnumValue | ...);
			    Archetype* archetype = new Archetype(components);
			    archetypes.push_back(archetype);
                (archetype->addComponent<Components>(), ...);
            }
    

        };
    }
