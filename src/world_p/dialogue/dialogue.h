#pragma once
#include "assets/serialize_structures/dialogue_serialize_data.h"
#include "renderer_p/image/bindable_image.h"

namespace rfct {
	struct frameContext;
	enum dialoguePartAnimation {
		Normal,
		Floating
	};
	class characterSpritesheet {
	public:
		characterSpritesheet(const std::string& characterName, const std::string& spritesheetName);
		~characterSpritesheet();
		bindableImage spriteSheetImage;
		int rowCount;
		int columnCount;
		std::map<std::string, spritesheetCycle> cycles;
		bool drawn = false;
	};
	struct dialogueParticipant {
		std::map<std::string, unique<characterSpritesheet>> spritesheets;
	};
	class dialogue {
	public:
		dialogue(const std::string& dialoguePath);
		void fullLoad();
		bool update(frameContext* ctx); // returns true if ended
		// text stuff
		void getDialogueData();
		void updateText(frameContext* ctx);
		// animation stuff
		void updateImage(frameContext* ctx);
		void changeSpritesheet();
		void onChangeFrame();
		void onChangeCycle();
	private:
		dialogueSerializeData m_serializeData;
		std::map<std::string, dialogueParticipant> participants;
	private:
		std::vector<dialoguePartAnimation> textAnimations;
		std::vector<std::string> text;

		bool loaded = false;
		float timeTillChangeOfIndexIsPossible;

		uint32_t lineChars;
		float currentTextFullAnimTime;
		float currentTextAnimTime;
		uint32_t nodeIndex;
		characterSpritesheet* currentSpritesheet = nullptr;
		std::string currentCycleName;
		float currentCycleReplayTime;
		float cyclePlayingTime;
		float framePlayingTime;
		bool currentCycleIsLooped;
		uint32_t cycleSpriteIndex = 0;
	};
};