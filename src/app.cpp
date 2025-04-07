#include "app.h"
#include "renderer_p\renderer.h"
#include "assets\assets_manager.h"
#include "world_p\world.h"
std::string rfct::reflectApplication::AssetsDirectory;
bool rfct::reflectApplication::shouldRender;

rfct::reflectApplication::reflectApplication(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR):
m_AssetsManager(AssetsDirectory), m_Renderer(RFCT_RENDERER_ARGUMENTS_VAR)
{
	world::getWorld().loadScene("");
    shouldRender = true;
	input::setInput(&m_Input);
#ifdef WINDOWS_BUILD
    update();
	renderer::getRen().showWindow();
	while (renderer::getRen().getWindow().pollEvents())
	{
		update();
	}
#endif
}
void rfct::reflectApplication::updateWindow(RFCT_NATIVE_WINDOW_ANDROID RFCT_NATIVE_WINDOW_ANDROID_VAR){
    m_Renderer.updateWindow(RFCT_NATIVE_WINDOW_ANDROID_VAR);
};
rfct::reflectApplication::~reflectApplication()
{
	RFCT_TRACE("app cleanup start");
	world::getWorld().cleanWorld();
}

void rfct::reflectApplication::update() {
	static auto previousTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> deltaTime = currentTime - previousTime;
	previousTime = currentTime;
	currentFrame = (currentFrame + 1) % RFCT_FRAMES_IN_FLIGHT;
	frameContext context = {
		.dt = deltaTime.count(),
		.frame = currentFrame,
		.scene = nullptr,
	};

	m_Input.pollEvents();
	updateGameplay(context);
	if (shouldRender) {
		renderer::getRen().render(context);
	};
}

void rfct::reflectApplication::updateGameplay(frameContext& ContextArg)
{
	world::getWorld().onUpdate(ContextArg);
}
