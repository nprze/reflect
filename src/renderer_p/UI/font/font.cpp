#include "font.h"
#include "assets/image_load.h"

rfct::font::font(const std::string& path) : 
	m_TextureAtlas(std::string(path).replace(path.size() - 3, 3, "png"))
{
	loadGlyphs(path, this);
}
