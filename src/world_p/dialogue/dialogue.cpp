#include "dialogue.h"
#include "assets/object_load.h"
#include "renderer_p/debug/debug_draw.h"
#include "input.h"
#include "renderer_p/renderer.h"

constexpr float waitBetweenLines = .02f;

rfct::dialogue::dialogue(const std::string& dialoguePath) {
	loadDialogue("dialogues/"+dialoguePath+".txt", &m_serializeData);
	nodeIndex = 0;
}

void rfct::dialogue::fullLoad() {
	RFCT_PROFILE_FUNCTION();
	for (auto& participantInfo : m_serializeData.participants) {
		dialogueParticipant participant = {};
		for (auto filename : participantInfo.spritesFilenames) {
			participant.spritesheets.emplace(
				filename,
				std::make_unique<characterSpritesheet>(participantInfo.name, filename)
			);
		}
		participants[participantInfo.name] = std::move(participant);
	}
	timeTillChangeOfIndexIsPossible = waitBetweenLines;
	loaded = true;

	displayPart.reserve(10);
}

void rfct::dialogue::visualUpdate(const frameContext* ctx) {
	RFCT_PROFILE_FUNCTION();
	updateText(ctx);
	updateImage(ctx);
}

bool rfct::dialogue::update(const frameContext* ctx) {
	RFCT_PROFILE_FUNCTION();
 	if (!loaded) return false;
	timeTillChangeOfIndexIsPossible -= ctx->dt;

	if (input::getInput().anyClicked) {
		input::getInput().anyClicked = false;
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

rfct::dialoguePartEffect getEffect(const std::string& name) {
	if (name == "=") return rfct::dialoguePartEffect::Normal;
	if (name == "f") return rfct::dialoguePartEffect::Floating;
	RFCT_CRITICAL("unknown effect");
}

void rfct::dialogue::getDialogueData() {
	displayPart.clear();

	lineChars = 0;

	std::string lineText =  m_serializeData.text[nodeIndex].dialogueText;
	std::size_t pos = 0;

	while (true) { // parse the serialize data
		dialoguePart part;
		std::size_t open = lineText.find('[', pos);
		if (open == std::string::npos) break;

		std::size_t close = lineText.find(']', open);
		if (close == std::string::npos) break;
		part.animation = getEffect(lineText.substr(open + 1, close - open - 1));
		std::size_t textStart = close + 1;
		std::size_t nextOpen = lineText.find('[', textStart);

		if (nextOpen == std::string::npos) {
			part.text = lineText.substr(textStart);
			lineChars += part.text.size();
			part.time = part.text.size() * 0.02f; 
			displayPart.push_back(part);
			break;
		}
		else {
			part.text = lineText.substr(textStart, nextOpen - textStart);
			pos = nextOpen;
			lineChars += part.text.size();
			part.time = part.text.size() * 0.02f;
			displayPart.push_back(part);
		}
	}
}

void rfct::dialogue::updateText(const frameContext* ctx) {
	RFCT_PROFILE_FUNCTION();
	if (displayPart.size() == 0) getDialogueData();

	float endX = 0.f;
	for (uint32_t i = 0; i < displayPart.size();i++) {
		dialoguePart singleEffectBit = displayPart[i];

		endX = debugDraw::drawText(singleEffectBit.text, { endX, 250 }, 0.2f);
	}
}

void rfct::dialogue::updateImage(const frameContext* ctx) {
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

	renderer::getRen().getUIPipeline().addImage({ 0, 0 }, { 100, 100 }, &(currentSpritesheet->spriteSheetImage), texMin, texMax);
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
