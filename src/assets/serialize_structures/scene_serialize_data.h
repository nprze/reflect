#pragma once
#include <string>
#include <glm/glm.hpp>
namespace rfct {
    enum SpikeDirection {
        SpikeDirUp,
        SpikeDirDown,
        SpikeDirRight,
        SpikeDirLeft
    };
    inline glm::vec2 getMoveDir(SpikeDirection dir) {
        glm::vec2 returnVal = {0,0};
        if (dir <= SpikeDirection::SpikeDirDown) {
            returnVal.y = (dir == SpikeDirection::SpikeDirUp ? 1: -1);
        }
        else {
            returnVal.x = (dir == SpikeDirection::SpikeDirRight ? 1: -1);
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
        float interatcionRadius;
        std::string dialogueFile;
    };
    struct SpikeInfo {
        glm::vec2 min;
        glm::vec2 max;
        SpikeDirection dir;
    };
    struct BasicEnemyInfo {
        glm::vec2 position;
        glm::vec2 min;
        glm::vec2 max;
        std::string animation;
    };
    struct JumpBoosterInfo {
        glm::vec2 position;
    };
    struct DashRechargeInfo {
        glm::vec2 position;
    };
    struct SpawnPointInfo {
        glm::vec2 position;
    };
    struct TallGrassInfo {
        glm::vec2 position;
    };
	struct sceneSerializedData {
        int width, height;
        std::vector<rectangle> rectangles;
        std::vector<vineInfo> vines;
        std::vector<NPCInfo> npcs;
        std::vector<SpikeInfo> spikes;
        std::vector<BasicEnemyInfo> enemies;
        std::vector<JumpBoosterInfo> boosters;
        std::vector<DashRechargeInfo> dashRecharge;
        std::vector<SpawnPointInfo> spawnPoints;
        std::vector<TallGrassInfo> tallGrass;
	};
}