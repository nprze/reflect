#pragma once
#include <glm/glm.hpp>
#include "assets/serialize_structures/frame_animation_serialize_data.h"
#include "renderer_p/rasterizer_pipeline/vertex.h"

namespace rfct {
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
        float oneBoneLenght = 0.f;
    };
    struct interactionDistanceComponent {
        float interationDistanceSquared = 0.f;
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
        glm::vec2 direction = glm::vec2(0.f);
    };
    struct sinusoidFloatComponent {
        float amplitude = 0.f;
        float frequency = 0.f;
        float phase = 0.f;
    };
    struct smokeDisperseComponent {
        float fullLenght = 0.f;
        float currentProgress = 0.f;
    };
    struct kindlingParticleComponent {
        glm::vec2 direction = glm::vec2(0.f);
        float fullLenght = 0.f;
        float currentProgress = 0.f;
    };
    struct deathAnimParticle {
        glm::vec2 direction = glm::vec2(0.f);
        float currentProgress = 0.f;
    };
    struct enemyComponent {
        bool facingRight = true;
        frameAnimation walkFrameAnim;
        frameAnimation turnFrameAnim;
        frameAnimation dieFrameAnim;
        uint8_t animIndex; // 0- walk, 1-turn, 2- die
        float turningTime = 0.f;
        float timeSinceFrameChanged = 0.f;
        uint8_t frameIndex = 0;
        size_t bufferOffset = 0;
    };
    struct enemyRayComponent {
        entity owner = entt::null;
    };
    struct jumpBoosterComponent {
        float timeSinceBoost = -1.f;
    };
    struct grassComponent {
        bool beenTouched = false;
        bool canBeFirst = true;
		float timeSinceTouched = 0.f;
        float sign = 1.f;
    };
}