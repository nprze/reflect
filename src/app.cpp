#include "app.h"
#include "renderer_p\renderer.h"
#include "assets\assets_manager.h"
std::string rfct::reflectApplication::AssetsDirectory;
bool rfct::reflectApplication::shouldRender;

rfct::reflectApplication::reflectApplication(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR):
m_AssetsManager(AssetsDirectory), m_Renderer(std::make_unique<renderer>(RFCT_RENDERER_ARGUMENTS_VAR)), m_Game()
{
	world::getWorld().loadWorld("");
    shouldRender = true;
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

	static auto previousTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> deltaTime = currentTime - previousTime;
	previousTime = currentTime;
	float dt = deltaTime.count();

	input::getInput().pollEvents();
	if (shouldRender) {
		m_Game.onUpdate(dt);
	}
	world::getWorld().onUpdate(dt);
	if (shouldRender) {
		renderer::getRen().render();
	};
}
