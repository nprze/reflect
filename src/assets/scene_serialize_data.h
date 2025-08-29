#pragma once
#include <string>
#include <glm/glm.hpp>
namespace rfct {
    enum SpikeDirection {
        up,
        down,
        right,
        left
    };
    inline glm::vec2 getMoveDir(SpikeDirection dir) {
        glm::vec2 returnVal = {0,0};
        if (dir <= SpikeDirection::down) {
            returnVal.y = (dir == SpikeDirection::up ? 1: -1);
        }
        else {
            returnVal.x = (dir == SpikeDirection::right ? 1: -1);
        }
        return returnVal;
    }
    struct rectangle {
        std::string color;
        glm::vec2 min;
        glm::vec2 max;
        std::string file;
    };
    struct vineInfo {
        glm::vec2 start;
        glm::vec2 end;
        int numEdges;
    };
    struct NPCInfo {
        glm::vec2 min;
        glm::vec2 max;
        float ineratcionRadius;
        std::string dialogueFile;
    };
    struct SpikeInfo {
        glm::vec2 min;
        glm::vec2 max;
        SpikeDirection dir;
    };
	struct sceneSerializedData {
        int width, height;
        std::vector<rectangle> rectangles;
        std::vector<vineInfo> vines;
        std::vector<NPCInfo> npcs;
        std::vector<SpikeInfo> spikes;
        glm::vec2 spawnPoint;
	};
}