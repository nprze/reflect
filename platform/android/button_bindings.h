#include <glm/glm.hpp>
#include "input.h"
#include "renderer_p/image/bindable_image.h"

namespace rfct {
    enum dir {
        DirUndefined = 0,
        right,
        rightTop,
        top,
        leftTop,
        left,
        leftBottom,
        bottom,
        rightBottom
    };
    struct theWierdButton {
        float timeBasicButtonActivated = 0.f;
        float timePossibleDash = 0.f;
        glm::vec2 position;
        float radius;
        float button;
        dir generalDirection = dir::DirUndefined;
        float timeSinceGeneralDirectionEstablished = 0.f;
        void dragCheck(const glm::vec2& point, const int& action, const int& pointerID, const float& dt, bool basicButtonActivated);
    };
    struct gameplayButtonBindings {
        static gameplayButtonBindings buttonBindings;
        button walkRight;
        button walkLeft;
        button jump;
        button menu;
        button hold;
        theWierdButton dash;

        void init();
        void clickCheck(const glm::vec2& point, const int& action, const int& pointerID, const frameContext* ctx);
        void updateInput(input& input) const;
        void drawButtons();
    };
    struct gameplayButtonRenderInfo{
        static gameplayButtonRenderInfo buttonRenderInfo;
        std::vector<unique<bindableImage>> Images;
        void bindImages();
    };
}