#include "font.h"
#include "assets/image_load.h"

const rfct::glyph* rfct::font::getGlyph(char character) const {
	auto it = glyphMap.find(character);
	if (it == glyphMap.end()) RFCT_CRITICAL("trying to get a glyph which isn't in the font");
	return &it->second;
}

rfct::font::font(const std::string& path)
	: m_TextureAtlas(std::string(path).replace(path.size() - 3, 3, "png")) {
    RFCT_PROFILE_FUNCTION();
	loadGlyphs(path, this);
	fontScale = 1.f/(1.5f * glyphMap['a'].height);
}

float rfct::font::getTextWidth(const std::string& text, float scale) {
    RFCT_PROFILE_FUNCTION();
	scale *= fontScale;
    float width = 0.f;
    const glyph* g;
    for (char c : text) {
        g = getGlyph(c);
        width += g->xadvance * scale;
    }
    width -= g->xadvance * scale;
    return width;
};