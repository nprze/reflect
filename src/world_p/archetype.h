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

        inline Archetype(ComponentEnum components)
            : componentsBitmask(components) {
            entities.reserve(RFCT_DEFAULT_ARCHETYPE_COMPONENTS_COUNT);
        }

        inline ~Archetype() {

            entities.clear();
            for (auto& [key, ptr] : componentMap) {
                delete static_cast<std::vector<ComponentBase>*>(ptr);
            }
        }

        inline ComponentEnum getComponentsBitmask() {
            return componentsBitmask;
        };
		template<typename... Components>
        inline size_t addEntity(Entity entity, Components&&... componentValues) {
            entities.emplace_back(entity);

            ((
                [&] {
                        auto* componentVector = static_cast<std::vector<std::decay_t<Components>>*>(componentMap[Components::EnumValue]);
                        componentVector->emplace_back(std::forward<Components>(componentValues));
                }()
                    ), ...);

            return entities.size() - 1;
        }


        template <typename Component>
        inline std::vector<Component>& getComponents(ComponentEnum componentEnum) {
            return *(static_cast<std::vector<Component>*>(componentMap[Component::EnumValue]));
        }
        

    private:
		template <typename ComponentType>
        inline void addComponent(ComponentEnum componentEnum) {
            RFCT_INFO("attempt");
            if (componentMap.find(componentEnum) != componentMap.end())return;
            RFCT_INFO("inside");
			componentMap[componentEnum] = new std::vector<ComponentType>();
            (static_cast<std::vector<ComponentType>*>(componentMap[componentEnum]))->reserve(RFCT_DEFAULT_ARCHETYPE_COMPONENTS_COUNT);
            componentsBitmask |= componentEnum;
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
        inline static Archetype* getArchetype(ComponentEnum requestedComponents) {
			for (Archetype* archetype : archetypes) {
				if (archetype->getComponentsBitmask() == requestedComponents) {
					return archetype;
				}
			}
			addArchetype<Components...>(requestedComponents);
			return archetypes.back();
        }
		template<typename... Components>
        inline static void addArchetype(ComponentEnum components) {
			Archetype* archetype = new Archetype(components);
			archetypes.push_back(archetype);
            (archetype->addComponent<Components>(Components::EnumValue), ...);
        }
        /*
        // Function to remove an archetype by type
        inline static void removeArchetype(Archetype* Arch) {
            auto it = std::find_if(archetypes.begin(), archetypes.end(),
                [Arch](const auto& pair) {
                    return pair.second.get() == Arch;  // Compare the raw pointer to the Archetype
                });

            if (it != archetypes.end()) {
                archetypes.erase(it);
            }
        }*/
    
    

    };
}
