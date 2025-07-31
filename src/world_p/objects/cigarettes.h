#pragma once
#include <glm/glm.hpp>

namespace rfct {
    struct frameContext;
    void onCollision_Cigarette_StaticObj(entity cigarette, entity collidedWith, glm::vec2 resolution);
    void updateCigarette(entity cigaretteEntity, const frameContext* fixedUpdateTimes);
}