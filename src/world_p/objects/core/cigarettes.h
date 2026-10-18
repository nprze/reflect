#pragma once
#include <glm/glm.hpp>
#include "world_p/objects/object_system.h"

namespace rfct {
    class scene;
    struct RfctFrameContext;
    struct cigarettes : public objectSystem {
        void initSystem();
        void spawnData(scene* s, sceneSerializedData* sd) {};
        void resetLevel(const RfctFrameContext* ctx);
        void updateVisuals(const RfctFrameContext* ctx);
        void updateSystem(RfctFrameContext* ctx);
        inline void cleanupSystem() {};

        void onDash(RfctFrameContext* fc, const entity entityPlayer, const bool facingRight);
        entity constructCigarette(const RfctFrameContext* fc, const entity entityPlayer, const bool facingRight);
    };
}