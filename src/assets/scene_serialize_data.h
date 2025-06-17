#pragma once
#include <string>
#include <glm/glm.hpp>
namespace rfct {
    struct rectangle {
        std::string color;
        glm::vec2 min;
        glm::vec2 max;
        std::string cutoff;
        std::string file;
    };
	struct sceneSerializedData {
        int width, height;
        std::vector<rectangle> rectangles;
	};
}