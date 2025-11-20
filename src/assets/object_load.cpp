#include "object_load.h"
#include "assets_utils.h"
#include "serialize_structures/dialogue_serialize_data.h"
#include <fstream>

void rfct::loadDialogue(const std::string& path, dialogueSerializeData* dialogueSerializedDataOut)
{
    std::ifstream file;
    if (!openAssetFile(path, &file)) {
        RFCT_CRITICAL("Could not open file:  {}", path);
    }
    enum class ParseState { None, Participants, DialogueText };
    ParseState state = ParseState::None;

    std::string line;
    dialogueParticipantSerializeData currentParticipant;

    auto trim = [](std::string& s) {
        auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
        s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
        };

    while (std::getline(file, line)) {
        std::string_view sv(line);
        trim(line);

        if (line.empty()) continue;

        // Version check
        if (sv.find("Version") != std::string::npos) {
            if (line != "Version 1.0") {
                RFCT_CRITICAL("Unsupported dialogue version: {}", line);
            }
            continue;
        }

        if (line == "Participants:") {
            state = ParseState::Participants;
            continue;
        }
        if (line == "DialogueText:") {
            if (!currentParticipant.name.empty()) {
                dialogueSerializedDataOut->participants.push_back(currentParticipant);
                currentParticipant = {};
            }
            state = ParseState::DialogueText;
            continue;
        }

        if (state == ParseState::Participants) {
            if (line.back() == ':') {
                // New participant
                if (!currentParticipant.name.empty()) {
                    dialogueSerializedDataOut->participants.push_back(currentParticipant);
                    currentParticipant = {};
                }
                currentParticipant.name = line.substr(0, line.size() - 1);
                trim(currentParticipant.name);
            }
            else {
                // Sprite filename
                currentParticipant.spritesFilenames.push_back(line);
            }
        }
        else if (state == ParseState::DialogueText) {
            // Format: {participant data}dialogue text
            auto openBrace = line.find('{');
            auto closeBrace = line.find('}');

            if (openBrace == std::string::npos || closeBrace == std::string::npos || closeBrace < openBrace) {
                RFCT_CRITICAL("Malformed dialogue text line: {}", line);
            }

            dialogueNodeSerializeData node;
            node.participantDataInBrackets = line.substr(openBrace + 1, closeBrace - openBrace - 1);
            trim(node.participantDataInBrackets);

            node.dialogueText = line.substr(closeBrace + 1);
            trim(node.dialogueText);

            dialogueSerializedDataOut->text.push_back(std::move(node));
        }
    }

    if (!currentParticipant.name.empty()) {
        dialogueSerializedDataOut->participants.push_back(currentParticipant);
    }

}

void rfct::loadDialogueSpriteSheet(const std::string& path, dialogueSpritesheetSerializeData* dialogueSpritesheetSerializedDataOut)
{
    std::ifstream file;
    if (!openAssetFile(path, &file)) {
        RFCT_CRITICAL("Could not open file:  {}", path);
    }

    std::string line;
    spritesheetCycle currentCycle;
    std::string currentCycleName;

    // First read row/column counts
    while (std::getline(file, line)) {
        if (line.empty())
            continue;

        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key == "ColumnCount:") {
            iss >> dialogueSpritesheetSerializedDataOut->columnCount;
        }
        else if (key == "RowCount:") {
            iss >> dialogueSpritesheetSerializedDataOut->rowCount;
        }
        else if (key == "CycleName:") {
            // Save the previous cycle if there was one
            if (!currentCycleName.empty()) {
                dialogueSpritesheetSerializedDataOut->cycles[currentCycleName] = currentCycle;
                currentCycle = spritesheetCycle(); // reset
            }
            iss >> currentCycleName;
        }
        else if (key == "Images:") {
            currentCycle.indices.clear();
            std::string token;
            while (iss >> token) {
                if (token.front() == '(' && token.back() == ')') {
                    token = token.substr(1, token.size() - 2); // strip ()
                    size_t commaPos = token.find(',');
                    if (commaPos != std::string::npos) {
                        int row = std::stoi(token.substr(0, commaPos));
                        int col = std::stoi(token.substr(commaPos + 1));
                        currentCycle.indices.emplace_back(row, col);
                    }
                }
            }
        }
        else if (key == "Repeat:") {
            iss >> currentCycle.repeat;
        }
        else if (key == "CycleTime:") {
            iss >> currentCycle.cycleTime;
        }
        else if (key == "Fallback:") {
            iss >> currentCycle.fallBack;
        }
    }

    // Store the last cycle if there was one
    if (!currentCycleName.empty()) {
        dialogueSpritesheetSerializedDataOut->cycles[currentCycleName] = currentCycle;
    }

}