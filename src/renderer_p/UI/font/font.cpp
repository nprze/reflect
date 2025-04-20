#include "font.h"

#include "assets/assets_manager.h"

rfct::font::font(const std::string& path) : m_TextureAtlas(std::string(path).replace(path.size() - 3, 3, "png"))
{
	AssetsManager::get().loadGlyphs(path, this);
}
