#pragma once
#include <glm/glm.hpp>
#include <map>

namespace rfct {
	struct dialogueParticipantSerializeData {
		std::string name;
		std::vector<std::string> spritesFilenames;
	};
	struct dialogueNodeSerializeData {
		std::string participantDataInBrackets;
		std::string dialogueText;
	};
	struct dialogueSerializeData {
		std::vector<dialogueParticipantSerializeData> participants;
		std::vector<dialogueNodeSerializeData> text;
	};
	struct spritesheetCycle {
		std::vector<glm::vec2> indices;
		int repeat;
		float cycleTime;
		std::string fallBack;
	};
	struct dialogueSpritesheetSerializeData {
		int rowCount;
		int columnCount;
		glm::vec2 backgroundStart;
		glm::vec2 backgroundEnd;
		float portraitOffset;
		std::map<std::string, spritesheetCycle> cycles;
	};
}