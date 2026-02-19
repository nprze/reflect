#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace rfct {
	struct blockSerializeData {
		std::string file;
		glm::vec2 min;
		glm::vec2 max;
	};
	struct worldSerializeData{
		std::vector<blockSerializeData> blocks;
	};
}