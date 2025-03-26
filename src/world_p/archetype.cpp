#include "archetype.h"
#include "app.h"
#include "components_util.h"
std::vector<rfct::Archetype*> rfct::Query::archetypes;
namespace rfct {
    void Archetype::removeEntity(size_t index) {
        if (index >= entities.size()) RFCT_CRITICAL("trying to remove entity out of range");
		world::getWorld().m_EntityLocations.back().locationIndex = index;

		std::swap(entities.at(index), entities.back());
		entities.pop_back();

        for (auto& [compType, compVecPtr] : componentMap) {
            switch (compType) {
                case ComponentEnum::nameComponent: {
                    auto* vec = static_cast<std::vector<nameComponent>*>(compVecPtr);
                    if (index < vec->size() - 1) {
                        std::swap(vec->at(index), vec->back());
                    }
                    vec->pop_back();
                    break;
                }
                case ComponentEnum::damageComponent: {
                    auto* vec = static_cast<std::vector<damageComponent>*>(compVecPtr);
                    if (index < vec->size() - 1) {
                        std::swap(vec->at(index), vec->back());
                    }
                    vec->pop_back();
                    break;
                }
                case ComponentEnum::healthComponent: {
                    auto* vec = static_cast<std::vector<healthComponent>*>(compVecPtr);
                    if (index < vec->size() - 1) {
                        std::swap(vec->at(index), vec->back());
                    }
                    vec->pop_back();
                    break;
                }
                default:
                    RFCT_CRITICAL("Couldn't delete entity components");
                    break;
            }
        }

    }
}