#pragma once
#include <glm/glm.hpp>

namespace rfct {
    struct frameContext;
    void initCigaretteVertices();
    void onCollision_Cigarette_StaticObj(entity cigarette, entity collidedWith, glm::vec2 resolution);
    void updateCigarette(entity cigaretteEntity, const frameContext* fixedUpdateTimes);
    entity constructCigarette(const frameContext* fc, const entity entityPlayer, const bool facingRight);
    void constructCigaretteBoundingBox(entity cigarette);
}