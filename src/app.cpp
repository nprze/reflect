#include "app.h"
#include "renderer_p\renderer.h"
#include "assets\assets_manager.h"
std::string rfct::reflectApplication::AssetsDirectory;
bool rfct::reflectApplication::shouldRender;

rfct::reflectApplication::reflectApplication(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR):
m_AssetsManager(AssetsDirectory), m_Renderer(std::make_unique<renderer>(RFCT_RENDERER_ARGUMENTS_VAR)),m_camera(glm::vec3(0.f, 0.f, 1.0f), glm::vec3(0), 45.f, renderer::getRen().getAspectRatio(), 0.f, 100.f), m_Scene(m_camera), m_Game()
{
 	m_GameWorld.loadWorld("");
    shouldRender = true;
	scene::setCurrentScene(&m_Scene);
	input::setInput(&m_Input);
#ifdef WINDOWS_BUILD
    render();
	renderer::getRen().showWindow();
	while (renderer::getRen().getWindow().pollEvents())
	{
		render();
	}
#endif
}
void rfct::reflectApplication::updateWindow(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR){
    m_Renderer->updateWindow(RFCT_NATIVE_WINDOW_ANDROID_VAR);
};
rfct::reflectApplication::~reflectApplication()
{
	RFCT_TRACE("app cleanup start");
}

void rfct::reflectApplication::render() {
	input::getInput().pollEvents();
	if (shouldRender) {
		m_Game.onUpdate();
	}
	m_Scene.getCurrentScene()->onUpdate();
	if (shouldRender) {
		renderer::getRen().render();
	};
}
