#include "scene_data.h"
#include "renderer_p\renderer.h"

rfct::scene* rfct::scene::m_currentScene;

void rfct::scene::Update()
{
	if (renderer::getRen().getDeviceWrapper().getSwapChain().framebufferResized) {
		m_camera.aspectRatio = renderer::getRen().getAspectRatio();
	}
}
