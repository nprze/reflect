#pragma once
#include "assets/dialogue_serialize_data.h"
#include "renderer_p/image/bindable_image.h"
#include "assets/dialogue_serialize_data.h"

namespace rfct {
	constexpr float waitBetweenLines = .5f;
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
	class dialogueParticipant {
	public:
		std::map<std::string, unique<characterSpritesheet>> spritesheets;
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
		float timeTillChangeOfIndexIsPossible;

		// text stuff
		void getDialogueData();
		void updateText(frameContext* ctx);

		float currentTextFullAnimTime;
		float currentTextAnimTime;
		std::vector<dialoguePartAnimation> textAnimations;
		std::vector<std::string> text;
		uint32_t lineChars;

		// animation stuff
		void updateImage(frameContext* ctx);
		void changeSpritesheet();
		void onChangeFrame();
		void onChangeCycle();

		characterSpritesheet* currentSpritesheet = nullptr;
		std::string currentCycleName;
		float currentCycleReplayTime;

		float cyclePlayingTime;
		float framePlayingTime;

		bool currentCycleIsLooped;

		uint32_t cycleSpriteIndex = 0;
	};
};