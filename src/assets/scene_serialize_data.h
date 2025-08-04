#pragma once
#include <string>
#include <glm/glm.hpp>
namespace rfct {
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
	struct sceneSerializedData {
        int width, height;
        std::vector<rectangle> rectangles;
        std::vector<vineInfo> vines;
	};
}