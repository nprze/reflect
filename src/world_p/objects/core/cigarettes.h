#pragma once
#include <glm/glm.hpp>
#include "world_p/objects/object_system.h"

namespace rfct {
    class scene;
    struct frameContext;
    struct cigarettes : public objectSystem {
        void initSystem();
        void spawnData(scene* s, sceneSerializedData* sd) {};
        void resetLevel(const frameContext* ctx);
        void updateVisuals(const frameContext* ctx);
        void updateSystem(frameContext* ctx);
        inline void cleanupSystem() {};

        void onDash(frameContext* fc, const entity entityPlayer, const bool facingRight);
        entity constructCigarette(const frameContext* fc, const entity entityPlayer, const bool facingRight);
    };
}