#pragma once
#include <string>
#include <glm/glm.hpp>
namespace rfct {
    struct rectangle {
        std::string color;
        glm::vec2 min;
        glm::vec2 max;
        int cutoff;
        std::string file;
    };
	struct sceneSerializedData {
        int width, height;
        std::vector<rectangle> rectangles;
	};
    enum cutoffValues {
        top                             = 1 << 0,
        right                           = 1 << 1,
        bottom                          = 1 << 2,
        left                            = 1 << 3,
        right_top_corner_top            = 1 << 4,
        right_top_corner_right          = 1 << 5,
        right_bottom_corner_right       = 1 << 6,
        right_bottom_corner_bottom      = 1 << 7,
        left_bottom_corner_bottom       = 1 << 8,
        left_bottom_corner_left         = 1 << 9,
        left_top_corner_left            = 1 << 10,
        left_top_corner_top             = 1 << 11
    };
}