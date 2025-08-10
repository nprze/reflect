#pragma once
#include "assets/dialogue_serialize_data.h"
#include "renderer_p/image/bindable_image.h"
#include "assets/dialogue_serialize_data.h"

namespace rfct {
	struct frameContext;
	class characterSpritesheet {
	public:
		characterSpritesheet(const std::string& characterName, const std::string& spritesheetName);
		bindableImage spriteSheetImage;
		int rowCount;
		int columnCount;
		std::map<std::string, spritesheetCycle> cycles;
	};
	class dialogueParticipant {
	public:
		std::map<std::string, characterSpritesheet> spritesheets;
	};
	class dialogue {
	public:
		dialogue(const std::string& dialoguePath);
		void fullLoad();
		bool update(frameContext* ctx); // returns true if ended
	private:
		dialogueSerializeData m_serializeData;
		std::map<std::string, dialogueParticipant> participants;
	private:
		uint32_t nodeIndex;
		bool loaded = false;
	};
};