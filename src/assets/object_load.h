#pragma once

namespace rfct {
	struct worldSerializeData;
	struct sceneSerializedData;
	struct dialogueSerializeData;
	struct dialogueSpritesheetSerializeData;

	// world
	void loadWorld(const std::string& path, worldSerializeData* out);
	// scene
	void loadScene(const std::string& path, sceneSerializedData* out);
	// dialogue
	void loadDialogue(const std::string& path, dialogueSerializeData* dialogueSerializedDataOut);
	void loadDialogueSpriteSheet(const std::string& path, dialogueSpritesheetSerializeData* dialogueSpritesheetSerializedDataOut);
}