#include "dialogue.h"
#include "assets/object_load.h"
#include "renderer_p/debug/debug_draw.h"
#include "input.h"
#include "renderer_p/renderer.h"

constexpr float waitBetweenLines = .5f;

rfct::dialogue::dialogue(const std::string& dialoguePath) {
	loadDialogue("dialogues/"+dialoguePath+".txt", &m_serializeData);
	RFCT_INFO("dialogue participant count: {}", m_serializeData.participants.size());
	nodeIndex = 0;
}

void rfct::dialogue::fullLoad() {
	RFCT_PROFILE_FUNCTION();
	for (auto& participantInfo : m_serializeData.participants) {
		dialogueParticipant participant = {};
		for (auto filename : participantInfo.spritesFilenames) {
			unique<characterSpritesheet> ss = std::make_unique<characterSpritesheet>(participantInfo.name, filename);
			participant.spritesheets.emplace(
				filename,
				std::move(ss)
			);
		}
		participants[participantInfo.name] = std::move(participant);
	}
	timeTillChangeOfIndexIsPossible = waitBetweenLines;
	loaded = true;
}

bool rfct::dialogue::update(frameContext* ctx) {
	RFCT_PROFILE_FUNCTION();
	if (!loaded) return false;
	timeTillChangeOfIndexIsPossible -= ctx->dt;

	updateText(ctx);
	updateImage(ctx);

	if (input::getInput().anyClicked && timeTillChangeOfIndexIsPossible <= 0.f) {
		nodeIndex++;
		timeTillChangeOfIndexIsPossible = waitBetweenLines;
		if (nodeIndex >= m_serializeData.text.size()) {
			return true;
		}
		getDialogueData();
		changeSpritesheet();
	}
	return false;
}

rfct::dialoguePartAnimation getAnim(const std::string& name) {
	if (name == "=") return rfct::dialoguePartAnimation::Normal;
	if (name == "f") return rfct::dialoguePartAnimation::Floating;
	RFCT_CRITICAL("unknown animation");
}

void rfct::dialogue::getDialogueData() {
	currentTextAnimTime = 0.f;
	text.clear();
	textAnimations.clear();

	lineChars = 0;

	std::string lineText =  m_serializeData.text[nodeIndex].dialogueText;
	std::size_t pos = 0;

	while (true) {
		std::size_t open = lineText.find('[', pos);
		if (open == std::string::npos) break;

		std::size_t close = lineText.find(']', open);
		if (close == std::string::npos) break;
		textAnimations.push_back(getAnim(lineText.substr(open + 1, close - open - 1)));
		std::size_t textStart = close + 1;
		std::size_t nextOpen = lineText.find('[', textStart);

		if (nextOpen == std::string::npos) {
			text.push_back(lineText.substr(textStart));
			lineChars += text.back().size();
			break;
		}
		else {
			text.push_back(lineText.substr(textStart, nextOpen - textStart));
			lineChars += text.back().size();
			pos = nextOpen; 
		}
	}

	// TODO: unhardcode this
	currentTextFullAnimTime = 2.f;
}

void rfct::dialogue::updateText(frameContext* ctx) {
	RFCT_PROFILE_FUNCTION();
	if (text.size() == 0) getDialogueData();

	currentTextAnimTime += ctx->dt;
	uint32_t charsToDisplay = lineChars * currentTextAnimTime / currentTextFullAnimTime;
	charsToDisplay = std::min(charsToDisplay, lineChars);

	uint32_t charsDisplayed = 0;
	float endX = 0.f;
	for (uint32_t i = 0; i < text.size();i++) {
		std::string part = text[i];
		if (charsDisplayed + part.size() > charsToDisplay) {
			// display only a part of part
			uint32_t allowedChars = charsToDisplay - charsDisplayed;
			endX = debugDraw::drawText(part.substr(0, allowedChars), {endX, 250}, 0.2f);
		}
		else {
			// display whole
			endX = debugDraw::drawText(part, { endX, 250 }, 0.2f);
		}
	}
}

void rfct::dialogue::updateImage(frameContext* ctx) {
	RFCT_PROFILE_FUNCTION();
	if (currentSpritesheet == nullptr) changeSpritesheet();
	spritesheetCycle currentCycle = currentSpritesheet->cycles[currentCycleName];

	cyclePlayingTime += ctx->dt;
	framePlayingTime += ctx->dt;


	if (framePlayingTime > currentCycle.cycleTime / currentCycle.indices.size()) {
		if (!currentCycleIsLooped && cyclePlayingTime > currentCycleReplayTime) {
			// cycle finished, fallback to another anim cycle
			currentCycleName = currentCycle.fallBack;
			currentCycle = currentSpritesheet->cycles.at(currentCycle.fallBack);
			currentCycleReplayTime = currentCycle.cycleTime;
			cycleSpriteIndex = 0;
			onChangeCycle();
		}
		else {
			if (!currentCycleIsLooped) {
				// animation contiune, change spriteIndex
				cycleSpriteIndex = (cycleSpriteIndex + 1) % currentCycle.indices.size();
				onChangeFrame();
			}
		}
	}

	// get current texture min max coords
	float oneOverRowCount = 1.f / currentSpritesheet->rowCount;
	float oneOverColumnCount = 1.f / currentSpritesheet->columnCount;
	glm::vec2 spriteSheetTextureCoords = currentCycle.indices[cycleSpriteIndex];
	glm::vec2 texMin = { spriteSheetTextureCoords.y * oneOverColumnCount, spriteSheetTextureCoords.x * oneOverRowCount };
	glm::vec2 texMax = { (spriteSheetTextureCoords.y+1) * oneOverColumnCount, (spriteSheetTextureCoords.x+1) * oneOverRowCount };

	renderer::getRen().getUIPipeline().addImage({ 10, 300 }, { 110, 400 }, &(currentSpritesheet->spriteSheetImage), texMin, texMax);
}

void rfct::dialogue::changeSpritesheet() {
	RFCT_PROFILE_FUNCTION();
	std::istringstream iss(m_serializeData.text[nodeIndex].participantDataInBrackets);
	std::vector<std::string> parts;
	std::string word;

	while (iss >> word) {
		parts.push_back(word);
	}

	currentSpritesheet = participants.at(parts[0]).spritesheets.at(parts[1]).get();
	participants.at(parts[0]).spritesheets.at(parts[1]).get()->drawn = true;

	currentCycleName = parts[2];
	if (parts[3] == "auto") {
		// TODO: unhardcode this
		currentCycleReplayTime = 2.f;
	}
	else {
		currentCycleReplayTime = std::stof(parts[3]);
	}

	cycleSpriteIndex = 0;

	onChangeCycle();
}

void rfct::dialogue::onChangeFrame() {
	framePlayingTime = 0.f;
}

void rfct::dialogue::onChangeCycle() {
	cyclePlayingTime = 0.f;
	onChangeFrame();
	currentCycleIsLooped = (currentSpritesheet->cycles[currentCycleName].fallBack == "");
}

rfct::characterSpritesheet::characterSpritesheet(const std::string& characterName, const std::string& spritesheetName): spriteSheetImage("dialogues/characters/"+characterName+"/"+spritesheetName+".png") {
	dialogueSpritesheetSerializeData sd;
	loadDialogueSpriteSheet("dialogues/characters/" + characterName + "/" + spritesheetName + ".txt", &sd);
	cycles = std::move(sd.cycles);
	rowCount = sd.rowCount;
	columnCount = sd.columnCount;
}

rfct::characterSpritesheet::~characterSpritesheet() {
	if (drawn) renderer::getRen().getUIPipeline().removeImage(&spriteSheetImage);
}
