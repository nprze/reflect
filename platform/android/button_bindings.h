#include <glm/glm.hpp>
#include "input.h"
#include "renderer_p/image/bindable_image.h"

namespace rfct {
    struct gameplayButtonBindings {
        static gameplayButtonBindings buttonBindings;
        button walkRight;
        button walkLeft;
        button jump;
        button menu;

        void init();
        void startEventParse();
        void clickCheck(const glm::vec2& point, const int& action, const int& pointerID);
        void updateInput(input& input) const;
        void drawButtons();
    };
    struct gameplayButtonRenderInfo{
        static gameplayButtonRenderInfo buttonRenderInfo;
        std::vector<unique<bindableImage>> Images;
        void bindImages();
    };
}