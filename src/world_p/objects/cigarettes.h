#pragma once
#include <glm/glm.hpp>

namespace rfct {
    class scene;
    struct frameContext;
    void initCigaretteVars(scene* scene);
    void cleanupCigarettes();
    void onCollision_Cigarette_StaticObj(entity cigarette, entity collidedWith, glm::vec2 resolution);
    void updateCigarettes(frameContext* ctx);
    void updateCigarettesMatrixes(const frameContext* ctx);
    entity constructCigarette(const frameContext* fc, const entity entityPlayer, const bool facingRight);
}