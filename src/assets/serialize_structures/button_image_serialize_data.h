#pragma once
#include <glm/glm.hpp>

namespace rfct {
	struct buttonCoordInfo {
		glm::vec2 released;
		glm::vec2 hold;
	};
	struct buttonImageSerializeData {
		std::string imagePath;
		std::string joystickImagePath;
		int imageRows;
		int imageColumns;
		glm::vec2 buttonSize;

		buttonCoordInfo joystick  = {{-1, -1}, {-1, -1}};
		buttonCoordInfo hold      = {{-1, -1}, {-1, -1}};
		buttonCoordInfo jump      = {{-1, -1}, {-1, -1}};
		buttonCoordInfo dash	  = {{-1, -1}, {-1, -1}};
		buttonCoordInfo menu      = {{-1, -1}, {-1, -1}};
	};
}