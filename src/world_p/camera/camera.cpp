#include "camera.h"
#include "renderer_p\renderer.h"
#include "world_p\world.h"
namespace rfct {
	void cameraComponentOnUpdate(float dt, const transformComponent& playertransform)
    {
        cameraComponent* cam = world::getWorld().getCurrentScene().getComponent<cameraComponent>(world::getWorld().getCurrentScene().camera);
        cam->aspectRatio = renderer::getRen().getAspectRatio();

        if (input::getInput().cameraXAxis || input::getInput().cameraYAxis || ((!input::getInput().xAxis) && (!input::getInput().yAxis))) {
            cam->position.x += dt * input::getInput().cameraXAxis;
            cam->position.y += dt * input::getInput().cameraYAxis;
        }
        else {
            cam->position.x = playertransform.position.x;
            cam->position.y = playertransform.position.y;
        }
        if (renderer::getRen().getDeviceWrapper().getSwapChain().framebufferResized) {
            cam->aspectRatio = renderer::getRen().getAspectRatio();
        }

    }
}