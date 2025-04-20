#include "app.h"
#include "world_p/world.h"
#include "assets/assets_manager.h"

bool rfct::reflectApplication::isAppMinimised;

rfct::reflectApplication::reflectApplication(RFCT_APP_ARGS):
m_Renderer(RFCT_RENDERER_ARGUMENTS_VAR)
{
	// app init
	input::getInput().init();
	isAppMinimised = false;
	registerComponents();


	world::getWorld().loadScene("");
#ifdef WINDOWS_BUILD
    update();
	renderer::getRen().getWindow().show();
	while (renderer::getRen().getWindow().pollEvents())
	{
		update();
	}
#endif
}
void rfct::reflectApplication::updateWindow(RFCT_APP_ARGS){
    m_Renderer.updateWindow(RFCT_NATIVE_WINDOW_ANDROID_VAR);
};
rfct::reflectApplication::~reflectApplication()
{
	RFCT_TRACE("app cleanup start");
	renderer::getRen().getDevice().waitIdle();
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
		.scene = &world::getWorld().getCurrentScene(),
	};

	input::getInput().pollEvents();
	if (!isAppMinimised) {
		updateGameplay(context);
		renderer::getRen().render(context);
	};
}

void rfct::reflectApplication::updateGameplay(frameContext& ContextArg)
{
	world::getWorld().onUpdate(ContextArg);
}
