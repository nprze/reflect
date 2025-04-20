#include "input.h"
#include <glm/glm.hpp>
#include "renderer_p/renderer.h"
#include "android_glue.h"

namespace rfct {
    struct button{
        glm::vec2 min;
        glm::vec2 max;
    };

    button buttons[2];
    input input::s_input;
    input::input():xAxis(0), yAxis(0), zAxis(0), cameraXAxis(0), cameraYAxis(0), cameraZAxis(0), windowExtent(nullptr)
    {
    }
    void input::init() {
        windowExtent = &(renderer::getRen().getWindow().extent);
        vk::Extent2D ext = *windowExtent;
        buttons[0].min = { 0,0 };
        buttons[0].max = { ext.width / 2,ext.height };
        buttons[1].min = { ext.width / 2,0 };
        buttons[1].max = { ext.width,ext.height };
    }
    void input::pollEvents() {
        xAxis=0;
        yAxis=0;
        zAxis=0;
        cameraXAxis=0;
        cameraYAxis=0;
        cameraZAxis=0;

        for (InputEvent event : InputQueue::eventQueue) {
            if (event.action == 1||event.action == 2) {
                glm::vec2 point = {event.x, event.y};

                if (point.x >= buttons[0].min.x && point.x <= buttons[0].max.x &&
                    point.y >= buttons[0].min.y && point.y <= buttons[0].max.y) {
                    cameraXAxis-=1;
                }
                else if (point.x >= buttons[1].min.x && point.x <= buttons[1].max.x &&
                         point.y >= buttons[1].min.y && point.y <= buttons[1].max.y) {
                    cameraXAxis+=1;
                }
            }
        }
    }
}