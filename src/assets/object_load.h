#pragma once

namespace rfct {
	struct sceneSerializedData;
	struct dialogueSerializeData;
	struct dialogueSpritesheetSerializeData;

	// scene
	void loadScene(const std::string& path, sceneSerializedData* out);

	// dialogue
	void loadDialogue(const std::string& path, dialogueSerializeData* dialogueSerializedDataOut);
	void loadDialogueSpriteSheet(const std::string& path, dialogueSpritesheetSerializeData* dialogueSpritesheetSerializedDataOut);
}