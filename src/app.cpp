#include "app.h"
#include "renderer_p\renderer.h"
#include "assets\assets_manager.h"
std::string rfct::reflectApplication::AssetsDirectory;

rfct::reflectApplication::reflectApplication(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR):
        m_AssetsManager(AssetsDirectory), m_Renderer(std::make_unique<renderer>(RFCT_RENDERER_ARGUMENTS_VAR)),m_camera(glm::vec3(0.f, 0.f, 1.0f), glm::vec3(0), 45.f, renderer::getRen().getAspectRatio(), 0.f, 100.f), m_Scene(m_camera), m_Game()
{
	scene::setCurrentScene(&m_Scene);
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
	m_Scene.getCurrentScene()->onUpdate();
    renderer::getRen().render();
}
