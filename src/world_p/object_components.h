#pragma once
#include <glm/glm.hpp>
#include "assets/frame_animation.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"

namespace flecs {
    class world;
    class entity;
}
namespace rfct {
    class scene;
    struct frameContext;

    struct vinePositionsComponent {
        std::vector<glm::vec2> previousPosition;
        std::vector<glm::vec2> positions;
    };

    struct vineVerticesComponent {
        std::vector<Vertex> vertices;
    };

    struct vineBasePositionsComponent {
        std::vector<glm::vec2> basePositions;
    };
    struct vineStateComponent {
        bool holdingToThis = false;
    };
    struct vineLenghtComponent {
        float oneBoneLenght;
    };
    struct interactionDistanceComponent {
        float interationDistanceSquared;
    };
    struct dialoguePathComponent {
        std::string dialoguePath;
    };

    struct cigaretteUpdateComponent {
        bool shouldBeUpdated = false;
        bool shouldSpawnSmoke = false;
        bool spawnedSmoke = false;
    };

    struct smokeFinishedComponent {
        bool hasSmokeFinished = false;
    };
    struct smokeParticleComponent {
        glm::vec2 direction;
    };
    struct sinusoidFloatComponent {
        float amplitude;
        float frequency;
        float phase;
    };
    struct smokeDisperseComponent {
        float fullLenght;
        float currentProgress;
    };

    struct kindlingParticleComponent {
        glm::vec2 direction;
        float fullLenght;
        float currentProgress;
    };

    struct deathAnimParticle {
        glm::vec2 direction;
        float currentProgress;

    };

    struct enemyComponent {
        bool facingRight = true;
        frameAnimation walkFrameAnim;
        frameAnimation turnFrameAnim;
        frameAnimation dieFrameAnim;
        uint8_t animIndex; // 0- walk, 1-turn, 2- die
        float turningTime = 0.f;

        float timeSinceFrameChanged = 0.f;
        uint8_t frameIndex;
        size_t bufferOffset;
    };
    struct jumpBoosterComponent {
        float timeSinceBoost = -1.f;

    };
    struct enemyRayComponent {
        entity owner;
    };

}