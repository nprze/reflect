#include "input.h"
#include <glm/glm.hpp>
#include "renderer_p/renderer.h"
#include "android_glue.h"
#include "button_bindings.h"
namespace rfct {

    input input::s_input;
    input::input():walk(0), jump(0), windowExtent(nullptr)
    {
    }
    void input::init() {
        gameplayButtonBindings::buttonBindings.init();
    }
    void input::drawButtons(){
        gameplayButtonBindings::buttonBindings.drawButtons();
    }
    void input::pollAndParseEvents(frameContext* context) {
        for (InputEvent event : InputQueue::eventQueue) {
            glm::vec2 point = {event.x, event.y};
            gameplayButtonBindings::buttonBindings.clickCheck(point, event.action, event.pointerID, context);
        }
        gameplayButtonBindings::buttonBindings.updateInput(s_input);
    }
}