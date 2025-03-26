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
			RFCT_REMOVE_COMPONENT_AT_INDEX(compVecPtr, compType, index);
		}

    }
}