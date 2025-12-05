#include "font.h"
#include "assets/image_load.h"

rfct::font::font(const std::string& path) : 
	m_TextureAtlas(std::string(path).replace(path.size() - 3, 3, "png"))
{
	loadGlyphs(path, this);
	fontScale = 1.f/(1.5f * glyphMap['a'].height);
}

float rfct::font::getTextWidth(const std::string& text, float scale)
{
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