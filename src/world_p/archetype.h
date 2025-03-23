#pragma once
#include <vector>
#include <tuple>
#include <stdexcept>
#include "components.h"

namespace rfct {

    struct BaseArchetype {
        virtual ~BaseArchetype() = default;

        virtual void* getComponentRaw(size_t index, size_t typeHash) = 0;

        template <typename Component>
        Component& getComponent(size_t index) {
            void* rawPtr = getComponentRaw(index, typeid(Component).hash_code());
            if (!rawPtr) throw std::runtime_error("Invalid component access");
            return *static_cast<Component*>(rawPtr);
        }
    };

    template<typename... Components>
    struct Archetype : public BaseArchetype {
        ComponentEnum componentsBitmask;
        std::vector<Entity> entities;
        std::tuple<std::vector<Components>...> componentArrays;

        inline Archetype(size_t capacity = RFCT_DEFAULT_ARCHETYPE_COMPONENTS_COUNT)
            : componentsBitmask(ComponentEnum::None) {
            entities.reserve(capacity);
            (std::get<std::vector<Components>>(componentArrays).reserve(capacity), ...);
        }

        inline size_t addEntity(Entity entity, Components&&... components) { // returns the index (row)
            entities.emplace_back(entity);
            (std::get<std::vector<Components>>(componentArrays).emplace_back(std::forward<Components>(components)), ...);
			return entities.size() - 1;
        }

        template <typename Component>
        inline std::vector<Component>& getComponents() {
            return std::get<std::vector<Component>>(componentArrays);
        }

        inline void removeEntity(size_t index) {
            if (index >= entities.size()) return;

            size_t last = entities.size() - 1;
            if (index != last) {
                entities[index] = std::move(entities[last]);
                (std::swap(std::get<std::vector<Components>>(componentArrays)[index],
                    std::get<std::vector<Components>>(componentArrays)[last]), ...);
            }

            entities.pop_back();
            (std::get<std::vector<Components>>(componentArrays).pop_back(), ...);
        }

        virtual void* getComponentRaw(size_t index, size_t typeHash) override {
            void* result = nullptr;
            ((typeHash == typeid(Components).hash_code() ?
                result = static_cast<void*>(&std::get<std::vector<Components>>(componentArrays).at(index)) : result), ...);
            return result;
        }
    };

    struct Query {
        static std::vector<BaseArchetype*> archetypes;

        inline static void cleanUp() {
            for (auto archetype : archetypes) {
                delete archetype;
            }
        }

        template <typename... Components>
        inline static Archetype<Components...>* getArchetype() {
            for (auto archetype : archetypes) {
                if (auto* typedArchetype = dynamic_cast<Archetype<Components...>*>(archetype)) {
                    return typedArchetype;
                }
            }
            addArchetype<Components...>();
            return dynamic_cast<Archetype<Components...>*>(archetypes.back());
        }

        template <typename... Components>
        inline static void addArchetype() {
            archetypes.push_back(new Archetype<Components...>(RFCT_DEFAULT_ARCHETYPE_COMPONENTS_COUNT));
        }
    };
}
