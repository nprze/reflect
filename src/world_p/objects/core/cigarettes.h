#pragma once
#include <glm/glm.hpp>
#include "world_p/objects/object_system.h"

namespace rfct {
    class scene;
    struct frameContext;

    struct cigarettes : public objectSystem {
        void initSystem() override;
        void spawnData(scene* s, sceneSerializedData* sd) override;
        void resetLevel(const frameContext* ctx) override;
        //void onLevelSwitch(scene* scen) override;
        void updateVisuals(const frameContext* ctx) override;
        void updateSystem(frameContext* ctx) override;
        inline void cleanupSystem() override {};

        void onDash(frameContext* fc, const entity entityPlayer, const bool facingRight);

    private:
        entity constructCigarette(const frameContext* fc, const entity entityPlayer, const bool facingRight);
    };
}