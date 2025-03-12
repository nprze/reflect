#include "app.h"
#include "renderer_p\renderer.h"
#include "assets\assets_manager.h"
std::string rfct::reflectApplication::AssetsDirectory;

rfct::reflectApplication::reflectApplication(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR):
        m_AssetsManager(AssetsDirectory), m_Renderer(std::make_unique<renderer>(RFCT_RENDERER_ARGUMENTS_VAR)), m_Game()
{
#ifdef WINDOWS_BUILD
    render();
	renderer::getRen().showWindow();
	while (renderer::getRen().getWindow().pollEvents())
	{
		render();
	}
#endif
}

void rfct::reflectApplication::render() {
	m_Game.onUpdate();
    renderer::getRen().render();
}
