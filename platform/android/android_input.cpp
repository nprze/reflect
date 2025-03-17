#include "input.h"
#include "renderer_p\renderer.h"
#include "glm\glm.hpp"
#include "android_glue.h"
namespace rfct {
    struct button{
        glm::vec2 min;
        glm::vec2 max;
    };

    button buttons[2];
    input* input::s_input;
    input::input():xAxis(0), yAxis(0), zAxis(0), cameraXAxis(0), cameraYAxis(0), cameraZAxis(0), windowExtent(renderer::getRen().getWindow().extent)
    {
        s_input = this;
        buttons[0].min = {0,0};
        buttons[0].max = {windowExtent.width/2,windowExtent.height};
        buttons[1].min = {windowExtent.width/2,0};
        buttons[1].max = {windowExtent.width,windowExtent.height};
    }
    void input::pollEvents() {
        xAxis=0;
        yAxis=0;
        zAxis=0;
        cameraXAxis=0;
        cameraYAxis=0;
        cameraZAxis=0;

        for (InputEvent event : InputQueue::eventQueue) {
            if (event.action == 1||event.action == 2) { // Assuming action == 1 is a click
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