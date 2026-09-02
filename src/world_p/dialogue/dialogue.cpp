#include "dialogue.h"
#include "assets/object_load.h"
#include "renderer_p/debug/debug_draw.h"
#include "input.h"
#include "renderer_p/renderer.h"

constexpr float waitBetweenLines = .02f;
constexpr float defaultFontScale = 0.2f;

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
	timeTillChangeOfIndexIsPossible = 0.3f;
	loaded = true;

	displayPart.reserve(10);
}

void rfct::dialogue::visualUpdate(const frameContext* ctx) {
	RFCT_PROFILE_FUNCTION();
	updateBackground(ctx);
	updateText(ctx);
	updateImage(ctx);
}

bool rfct::dialogue::update(const frameContext* ctx) {
	RFCT_PROFILE_FUNCTION();
 	if (!loaded) return false;
	timeTillChangeOfIndexIsPossible -= ctx->dt;

	if (input::getInput().anyClicked && timeTillChangeOfIndexIsPossible < 0.f) {
		input::getInput().anyClicked = false;
		if (dialoguePartNodeIndex < displayPart.size()) {
			dialoguePartNodeIndex = displayPart.size();

			spritesheetCycle currentCycle = currentSpritesheet->cycles[currentCycleName];
			while (currentCycle.fallBack != "") {
				currentCycleName = currentCycle.fallBack;
				currentCycle = currentSpritesheet->cycles.at(currentCycleName);
			}
			currentCycleFullPlayTime = currentCycle.cycleTime;
			onChangeCycle();
		}
		else {
			nodeIndex++;
			timeTillChangeOfIndexIsPossible = waitBetweenLines;
			if (nodeIndex >= m_serializeData.text.size()) {
				return true;
			}
			getDialogueData();
			changeSpritesheet();
		}
	}
	return false;
}

rfct::dialoguePartEffect getEffect(const std::string& name) {
	if (name == "=") return rfct::dialoguePartEffect::Normal;
	if (name == "f") return rfct::dialoguePartEffect::Floating;
	RFCT_CRITICAL("unknown effect");
}

void rfct::dialogue::getDialogueData() {
	RFCT_PROFILE_FUNCTION();
	displayPart.clear();

	std::string lineText =  m_serializeData.text[nodeIndex].dialogueText;
	dialoguePartNodeIndex = 0;
	displayPartPlayingTime = 0.f;
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
			part.time = part.text.size() * (part.animation == Normal ? 0.02f : 0.1f);
			part.singleCharTime = part.time / part.text.size();
			displayPart.push_back(part);
			break;
		}
		else {
			part.text = lineText.substr(textStart, nextOpen - textStart);
			pos = nextOpen;
			part.time = part.text.size() * (part.animation == Normal ? 0.02f : 0.4f);
			part.singleCharTime = part.time / part.text.size();
			displayPart.push_back(part);
		}
	}
}

void rfct::dialogue::updateText(const frameContext* ctx) {
	RFCT_PROFILE_FUNCTION();
	if (displayPart.size() == 0) getDialogueData();

	float endX = textLeftOffsetInPixel;
	float fontHeightInPixel = RfctRenderer::getRen().getUIPipeline().getDefaultFont()->getFontHeight(defaultFontScale);
	float textOffsetTop = bgOffsetInPixel.y + (bgSizeInPixel.y * 0.5f) - (fontHeightInPixel);

	for (uint32_t i = 0; i < dialoguePartNodeIndex; i++) {
		dialoguePart singleEffectBit = displayPart[i];
		endX = debugDraw::drawText(singleEffectBit.text, { endX, textOffsetTop }, defaultFontScale);
	}
	if (dialoguePartNodeIndex < displayPart.size()) {
		displayPartPlayingTime += ctx->dt;
		dialoguePart singleEffectBit = displayPart[dialoguePartNodeIndex];
		uint32_t charsToDisplay = (uint32_t)(displayPartPlayingTime / singleEffectBit.singleCharTime);
		float animMult = std::fmod(displayPartPlayingTime, singleEffectBit.singleCharTime) / singleEffectBit.singleCharTime; // (0, 1)
		animMult = std::sin(2.f * 3.24f * animMult) * (1 - (animMult * animMult));


		endX = debugDraw::drawText(singleEffectBit.text.substr(0, charsToDisplay), { endX, textOffsetTop }, defaultFontScale);
		endX = debugDraw::drawText(singleEffectBit.text.substr(charsToDisplay, 1), { endX, textOffsetTop + (5 * animMult) }, defaultFontScale); // first char is animated
		RFCT_ASSERT(endX - textLeftOffsetInPixel < maxTextWidthInPixel)
		if (displayPartPlayingTime > displayPart[dialoguePartNodeIndex].time) {
			displayPartPlayingTime = 0.f;
			dialoguePartNodeIndex++;
		}
	}
}

void rfct::dialogue::updateImage(const frameContext* ctx) {
	RFCT_PROFILE_FUNCTION();
	if (currentSpritesheet == nullptr) changeSpritesheet();
	spritesheetCycle currentCycle = currentSpritesheet->cycles[currentCycleName];

	cyclePlayingTime += ctx->dt;
	framePlayingTime += ctx->dt;

	if (framePlayingTime > currentCycle.cycleTime / currentCycle.indices.size()) {
		if (!currentCycleIsLooped && cyclePlayingTime > currentCycleFullPlayTime) {
			// cycle finished, fallback to another anim cycle
			currentCycleName = currentCycle.fallBack;
			currentCycle = currentSpritesheet->cycles.at(currentCycleName);
			currentCycleFullPlayTime = currentCycle.cycleTime;
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

	RfctRenderer::getRen().getUIPipeline().addImage(bgOffsetInPixel + glm::vec2{ portraitOffsetInPixel, portraitOffsetInPixel }, 
		bgOffsetInPixel + glm::vec2{ portraitOffsetInPixel + portraitSizeInPixel, portraitOffsetInPixel + portraitSizeInPixel }, & (currentSpritesheet->spriteSheetImage), texMin, texMax);
}

void rfct::dialogue::updateBackground(const frameContext* ctx) {
	RFCT_PROFILE_FUNCTION();
	if (currentSpritesheet == nullptr) changeSpritesheet();
	glm::vec2 backgroundBegin = currentSpritesheet->backgroundBegin;
	glm::vec2 backgroundEnd = currentSpritesheet->backgroundEnd;

	float backgroundAspectRatio = (backgroundEnd.y + 1 - backgroundBegin.y) / (backgroundEnd.x + 1 - backgroundBegin.x); // width / height

	vk::Extent2D winExt = RfctRenderer::getRen().getExtent();

	bgSizeInPixel = { winExt.width * 0.7f, winExt.width * 0.7f / backgroundAspectRatio };
	bgOffsetInPixel = { winExt.width * 0.15f, winExt.height * 0.15f };
	portraitOffsetInPixel = currentSpritesheet->portraitOffset * bgSizeInPixel.y;
	portraitSizeInPixel = bgSizeInPixel.y - 2 * portraitOffsetInPixel;
	textLeftOffsetInPixel = bgOffsetInPixel.x + portraitSizeInPixel + 4 * portraitOffsetInPixel;
	maxTextWidthInPixel = bgSizeInPixel.x + bgOffsetInPixel.x - (textLeftOffsetInPixel + 2 * portraitOffsetInPixel);

	// get current texture min max coords
	float oneOverRowCount = 1.f / currentSpritesheet->rowCount;
	float oneOverColumnCount = 1.f / currentSpritesheet->columnCount;

	glm::vec2 texMin = { backgroundBegin.y * oneOverColumnCount, backgroundBegin.x * oneOverRowCount };
	glm::vec2 texMax = { (backgroundEnd.y + 1) * oneOverColumnCount, (backgroundEnd.x + 1) * oneOverRowCount };

	RfctRenderer::getRen().getUIPipeline().addImage(bgOffsetInPixel, { bgOffsetInPixel.x + bgSizeInPixel.x, bgOffsetInPixel.y + bgSizeInPixel.y }, &(currentSpritesheet->spriteSheetImage), texMin, texMax);
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
		currentCycleFullPlayTime = 2.f;
	}
	else {
		currentCycleFullPlayTime = std::stof(parts[3]);
	}

	onChangeCycle();
}

void rfct::dialogue::onChangeFrame() {
	framePlayingTime = 0.f;
}

void rfct::dialogue::onChangeCycle() {
	cycleSpriteIndex = 0;
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
	backgroundBegin = sd.backgroundStart;
	backgroundEnd = sd.backgroundEnd;
	portraitOffset = sd.portraitOffset;
}

rfct::characterSpritesheet::~characterSpritesheet() {
	if (drawn) RfctRenderer::getRen().getUIPipeline().removeImage(&spriteSheetImage);
}
