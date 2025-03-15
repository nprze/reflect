#include "scene_data.h"
#include "renderer_p\renderer.h"
#include "input.h"


rfct::scene* rfct::scene::m_currentScene;

void rfct::scene::Update()
{
    static auto previousTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> deltaTime = currentTime - previousTime;

    previousTime = currentTime;

	m_camera.position.x += deltaTime.count() * input::getInput().cameraXAxis;
	m_camera.position.y += deltaTime.count() * input::getInput().cameraYAxis;
	if (renderer::getRen().getDeviceWrapper().getSwapChain().framebufferResized) {
		m_camera.aspectRatio = renderer::getRen().getAspectRatio();
	}
}
