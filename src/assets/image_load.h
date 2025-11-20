#pragma once

namespace rfct {
	class image;
	class font;
	struct buttonImageSerializeData;

	void loadImage(const std::string& path, image* imageOut);
	void createDummyImage(image* imageOut);
	void loadGlyphs(const std::string& path, font* fontOut);
	void loadButtonImage(const std::string& path, buttonImageSerializeData* buttonImageSerializedDataOut);
}