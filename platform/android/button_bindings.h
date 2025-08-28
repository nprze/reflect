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
    struct joystick {
        bindableImage* joystickBg;
        glm::vec2 currentBttnPos;
        glm::vec2 position;
        float drawRadius;
        float intractionRadius;

        glm::vec2 joystickMiddleHalfSize;

        glm::vec2 joystickMiddleImageMin;
        glm::vec2 joystickMiddleImageMax;

        dir dashDirection = dir::DirUndefined;
        dir moveDirection = dir::DirUndefined;
        bool isTriggered;

        std::unordered_set<int> activePointers;
        glm::vec2 lastTouchPos = { -1, -1 };
        void update(const glm::vec2& point, const int& action, const int& pointerID, const float& dt);
        void updateDir();
    };
    struct gameplayButtonBindings {
        static gameplayButtonBindings buttonBindings;
        button jump;
        button menu;
        button hold;
        button dashBttn;
        joystick movement;

        void init();
        void clickCheck(const glm::vec2& point, const int& action, const int& pointerID, const frameContext* ctx);
        void updateInput(input& input) const;
        void drawButtons();
    };
    struct gameplayButtonRenderInfo{
        static gameplayButtonRenderInfo buttonRenderInfo;
        unique<bindableImage> Image;
        unique<bindableImage> JoystickImage;
        void bindImages(const std::string& path); // path is to a .txt file describing the button atlas image
    };
}