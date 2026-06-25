#pragma once
#include "assets/serialize_structures/dialogue_serialize_data.h"
#include "renderer_p/image/bindable_image.h"

namespace rfct {
	struct frameContext;
	enum dialoguePartEffect {
		Normal,
		Floating
	};
	struct dialoguePart {
		std::string text;
		dialoguePartEffect animation;
		float time;
		float singleCharTime;
	};
	class characterSpritesheet {
	public:
		characterSpritesheet(const std::string& characterName, const std::string& spritesheetName);
		~characterSpritesheet();
		bindableImage spriteSheetImage;
		int rowCount;
		int columnCount;
		std::map<std::string, spritesheetCycle> cycles;
		glm::vec2 backgroundBegin;
		glm::vec2 backgroundEnd;
		bool drawn = false;
	};
	struct dialogueParticipant {
		std::map<std::string, unique<characterSpritesheet>> spritesheets;
	};
	class dialogue {
	public:
		dialogue(const std::string& dialoguePath);
		void fullLoad();
		void visualUpdate(const frameContext* ctx);
		bool update(const frameContext* ctx); // returns true if ended
		void getDialogueData();
		void updateText(const frameContext* ctx);
		void updateImage(const frameContext* ctx);
		void updateBackground(const frameContext* ctx);
		void changeSpritesheet();
		void onChangeFrame();
		void onChangeCycle();
	private:
		dialogueSerializeData m_serializeData;
		std::map<std::string, dialogueParticipant> participants;
	private:
		std::vector<dialoguePart> displayPart; // one part is a single part with one effect.

		bool loaded = false;
		float timeTillChangeOfIndexIsPossible = 0.f;
		uint32_t nodeIndex;
		// text
		uint32_t dialoguePartNodeIndex;
		float displayPartPlayingTime;
		// image
		characterSpritesheet* currentSpritesheet = nullptr;
		std::string currentCycleName;
		float currentCycleReplayTime;
		float cyclePlayingTime;
		float framePlayingTime;
		bool currentCycleIsLooped;
		uint32_t cycleSpriteIndex = 0;
	};
};