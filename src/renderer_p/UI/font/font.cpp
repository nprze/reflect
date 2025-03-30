#include "font.h"
#include "renderer_p\renderer.h"
rfct::font::font(std::string path) : m_TextureAtlas("fonts/jetbrainsMono-medium.png")
{
	renderer::getRen().getAssetsManager()->loadGlyphs(path, this);
}
