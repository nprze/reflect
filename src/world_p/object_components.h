#pragma once
#include <glm/glm.hpp>

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
    struct vineStateComponent {
        bool holdingToThis = false;
    };
    struct vineLenghtComponent {
        float oneBoneLenght;
    };
    struct interactionDistanceComponent {
        float inetrationDistanceSquared;
    };
    struct dialoguePathComponent {
        std::string dialoguePath;
    };
}