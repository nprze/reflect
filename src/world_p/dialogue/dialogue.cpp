#include "dialogue.h"
#include "assets/assets_manager.h"
#include "context.h"
#include "renderer_p/debug/debug_draw.h"
#include "input.h"
#include "renderer_p/renderer.h"

rfct::dialogue::dialogue(const std::string& dialoguePath)
{
	AssetsManager::get().loadDialogue("dialogues/"+dialoguePath+".txt", &m_serializeData);
	RFCT_INFO("dialogue participant count: {}", m_serializeData.participants.size());
	nodeIndex = 0;
}

void rfct::dialogue::fullLoad()
{
	for (auto& participantInfo : m_serializeData.participants) {
		dialogueParticipant participant = {};
		for (auto filename : participantInfo.spritesFilenames) {
			participant.spritesheets.emplace(
				filename,
				characterSpritesheet(participantInfo.name, filename)
			);

		}
		participants[participantInfo.name] = std::move(participant);
	}
	loaded = true;
}

bool rfct::dialogue::update(frameContext* ctx) {
	if (!loaded) return false;
	debugDraw::drawText(m_serializeData.text[nodeIndex].dialogueText, {10, 250}, 0.2f);
	std::istringstream iss(m_serializeData.text[nodeIndex].participantDataInBrackets);
	std::vector<std::string> parts;
	std::string word;

	while (iss >> word) {
		parts.push_back(word);
	}

	renderer::getRen().getUIPipeline().addImage({ 10, 300 }, { 210, 400 }, &participants.at(parts[0]).spritesheets.at(parts[1]).spriteSheetImage);
	if (input::getInput().anyClicked) {
		nodeIndex++;
		if (nodeIndex >= m_serializeData.text.size()) {
			return true;
		}
	}
	return false;
	
}

rfct::characterSpritesheet::characterSpritesheet(const std::string& characterName, const std::string& spritesheetName): spriteSheetImage("dialogues/characters/"+characterName+"/"+spritesheetName+".png")
{
	dialogueSpritesheetSerializeData sd;
	AssetsManager::get().loadDialogueSpriteSheet("dialogues/characters/" + characterName + "/" + spritesheetName + ".txt", &sd);
	cycles = std::move(sd.cycles);
	rowCount = sd.rowCount;
	columnCount = sd.columnCount;
}
