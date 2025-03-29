#include "camera.h"
#include "renderer_p\renderer.h"
namespace rfct {
	void cameraComponentOnUpdate(float dt)
    {
        cameraComponent* cam = world::getWorld().getComponent<cameraComponent>(world::getWorld().camera);
        cam->aspectRatio = renderer::getRen().getAspectRatio();


        cam->position.x += dt * input::getInput().cameraXAxis;
        cam->position.y += dt * input::getInput().cameraYAxis;
        if (renderer::getRen().getDeviceWrapper().getSwapChain().framebufferResized) {
            cam->aspectRatio = renderer::getRen().getAspectRatio();
        }

    }
}