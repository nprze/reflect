#include "camera.h"
#include "renderer_p\renderer.h"
#include "world_p\world.h"
namespace rfct {
	void cameraComponentOnUpdate(float dt)
    {
        cameraComponent* cam = world::getWorld().getCurrentScene().getComponent<cameraComponent>(world::getWorld().getCurrentScene().camera);
        cam->aspectRatio = renderer::getRen().getAspectRatio();


        cam->position.x += dt * input::getInput().cameraXAxis;
        cam->position.y += dt * input::getInput().cameraYAxis;
        if (renderer::getRen().getDeviceWrapper().getSwapChain().framebufferResized) {
            cam->aspectRatio = renderer::getRen().getAspectRatio();
        }

    }
}