#include "button_bindings.h"
#include "renderer_p/renderer.h"
#include <glm/trigonometric.hpp>
#include <glm/gtc/constants.hpp>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

constexpr float timeForDashAfterBasicButtonReleased = 0.3f;
rfct::gameplayButtonBindings rfct::gameplayButtonBindings::buttonBindings;
rfct::gameplayButtonRenderInfo rfct::gameplayButtonRenderInfo::buttonRenderInfo;

void rfct::theWierdButton::dragCheck(const glm::vec2& point, const int& action, const int& pointerID, const float& dt, bool basicButtonActivated) {
    if (basicButtonActivated) {
        timeBasicButtonActivated = std::clamp(timeBasicButtonActivated + dt, 0.f, 0.2f);
    }
    else {
        if (timeBasicButtonActivated >= 0.02f) {
            timePossibleDash = timeForDashAfterBasicButtonReleased + dt;
        }
        timeBasicButtonActivated = 0.f;
    }
    timePossibleDash = std::clamp(timePossibleDash - dt, 0.f, timeForDashAfterBasicButtonReleased);
    generalDirection = dir::DirUndefined;
    if (timePossibleDash == 0.f) return;
    if (glm::distance(point, position) < radius && glm::distance(point, position) > button) {
        if (action == 2) {
            glm::vec2 direction = glm::normalize(point - position);
            float angle = atan2(direction.y, direction.x);


            dir directionEnum = DirUndefined;

            if (angle >= -M_PI / 8 && angle < M_PI / 8) {
                generalDirection = right;
            }
            else if (angle >= M_PI / 8 && angle < 3 * M_PI / 8) {
                generalDirection = rightBottom;
            }
            else if (angle >= 3 * M_PI / 8 && angle < 5 * M_PI / 8) {
                generalDirection = bottom;
            }
            else if (angle >= 5 * M_PI / 8 && angle < 7 * M_PI / 8) {
                generalDirection = leftBottom;
            }
            else if (angle >= 7 * M_PI / 8 || angle < -7 * M_PI / 8) {
                generalDirection = left;
            }
            else if (angle >= -7 * M_PI / 8 && angle < -5 * M_PI / 8) {
                generalDirection = leftTop;
            }
            else if (angle >= -5 * M_PI / 8 && angle < -3 * M_PI / 8) {
                generalDirection = top;
            }
            else if (angle >= -3 * M_PI / 8 && angle < -M_PI / 8) {
                generalDirection = rightTop;
            }

        }
    }
}

void rfct::gameplayButtonBindings::clickCheck(const glm::vec2 &point, const int& action, const int& pointerID, const frameContext* ctx){
    walkRight.updateIsClicked(point, action, pointerID);
    walkLeft.updateIsClicked(point, action, pointerID);
    jump.updateIsClicked(point, action, pointerID);
    menu.updateIsClicked(point, action, pointerID);
    hold.updateIsClicked(point, action, pointerID);
    dash.dragCheck(point, action, pointerID, ctx->dt, hold.isClicked);
}

void rfct::gameplayButtonBindings::updateInput(rfct::input &input) const{
    input.walk = 0;
    input.jump = 0;
    input.openMenu = false;
    input.dashDefault = 0;
    input.dashX = 0;
    input.dash45up = 0;
    input.dash45down = 0;
    input.dashY = 0;
    input.hold = false;


    if (walkRight.isClicked){
        input.walk += 1;
    }
    if (walkLeft.isClicked){
        input.walk -= 1;
    }
    if (jump.isClicked){
        input.jump += 1;
    }
    if (menu.isClicked){
        input.openMenu = true;
    }
    if (hold.isClicked) {
        input.hold = true;
    }
    if (dash.generalDirection != dir::DirUndefined) {
        switch (dash.generalDirection) {
        case right:
            input.dashX = 1;
            break;
        case rightTop:
            input.dash45up = 1;
            break;
        case top:
            input.dashY = 1;
            break;
        case leftTop:
            input.dash45down = -1;
            break;
        case left:
            input.dashX = -1;
            break;
        case leftBottom:
            input.dash45up = -1;
            break;
        case bottom:
            input.dashY = -1;
            break;
        case rightBottom:
            input.dash45down = 1;
            break;
        default:
            break;
        }
    }
    
}

void rfct::gameplayButtonBindings::init() {

    vk::Extent2D windowExtent = renderer::getRen().getWindow().getExtent();
    // walk bttns size
    glm::vec2 walkBttnsSize = {((float)windowExtent.height) * 0.25f, ((float)windowExtent.height) * 0.25f };
    glm::vec2 jumpBttnsSize = {((float)windowExtent.height) * 0.15f, ((float)windowExtent.height) * 0.15f };
    // basic layout:
    // left: 10% height from bottom, 10% height from left
    float tenPercentHeight = ((float)windowExtent.height) * 0.10f;
    walkLeft.min = {tenPercentHeight, ((float)windowExtent.height) - walkBttnsSize.y - tenPercentHeight};
    walkLeft.max = walkLeft.min + walkBttnsSize;
    // right: 10% height from walk left, same height as walk left
    walkRight.min = {walkLeft.min.x + walkBttnsSize.x + tenPercentHeight, walkLeft.min.y};
    walkRight.max = walkRight.min + walkBttnsSize;
    // jump: 10% height from bottom, 30% height from right
    jump.min = {((float)windowExtent.width) - jumpBttnsSize.x - 3 * tenPercentHeight, ((float)windowExtent.height) - jumpBttnsSize.y - tenPercentHeight};
    jump.max = jump.min + jumpBttnsSize;
    // hold: 40% height from bottom, 20% height from right
    hold.min = { ((float)windowExtent.width) - jumpBttnsSize.x - 2 * tenPercentHeight, ((float)windowExtent.height) - jumpBttnsSize.y - 4 * tenPercentHeight };
    hold.max = hold.min + jumpBttnsSize;
    // menu aligned to right top corner, width and height 10% of window height
    menu.min = {windowExtent.width - tenPercentHeight, 0};
    menu.max = menu.min + glm::vec2(tenPercentHeight, tenPercentHeight);
   
    dash.position = { (hold.min + hold.max) * 0.5f };
    dash.radius = tenPercentHeight * 2.5;
    dash.button = (hold.max.x - hold.min.x) * 0.5f;

    gameplayButtonRenderInfo::buttonRenderInfo.bindImages();
}

void rfct::gameplayButtonBindings::drawButtons() {
    renderer::getRen().getUIPipeline().addImage(walkLeft.min, walkLeft.max, walkLeft.image);
    renderer::getRen().getUIPipeline().addImage(walkRight.min, walkRight.max, walkRight.image);
    renderer::getRen().getUIPipeline().addImage(jump.min, jump.max, jump.image);
    renderer::getRen().getUIPipeline().addImage(menu.min, menu.max, menu.image);
    renderer::getRen().getUIPipeline().addImage(hold.min, hold.max, hold.image);
    if (dash.timeBasicButtonActivated != 0.f) {
        float widthAndHeight = dash.radius - dash.button;
        // right
        renderer::getRen().getUIPipeline().addImage({ dash.position.x + dash.button, dash.position.y - (widthAndHeight * 0.5f) }, { dash.position.x + dash.radius, dash.position.y + (widthAndHeight * 0.5f) }, dash.image);
        // left
        renderer::getRen().getUIPipeline().addImage({ dash.position.x - dash.button, dash.position.y - (widthAndHeight * 0.5f) }, { dash.position.x - dash.radius, dash.position.y + (widthAndHeight * 0.5f) }, dash.image);
        // up
        renderer::getRen().getUIPipeline().addImage(
                { dash.position.x - (widthAndHeight * 0.5f), dash.position.y - dash.radius },
                { dash.position.x + (widthAndHeight * 0.5f), dash.position.y - dash.button },
                dash.imageUp
        );
        // down
        renderer::getRen().getUIPipeline().addImage(
                { dash.position.x - (widthAndHeight * 0.5f), dash.position.y + dash.radius },
                { dash.position.x + (widthAndHeight * 0.5f), dash.position.y + dash.button },
                dash.imageUp
        );
        float offset = dash.button  * (1/std::sqrt(2.0f));
        float offsetMax = dash.radius * 0.9f;
        // top-right (45°)
        renderer::getRen().getUIPipeline().addImage(
                { dash.position.x + offset, dash.position.y - offsetMax },
                { dash.position.x + offsetMax, dash.position.y - offset },
                dash.image45
        );
        renderer::getRen().getUIPipeline().addImage(
                { dash.position.x + offset, dash.position.y + offset },      // min
                { dash.position.x + offsetMax, dash.position.y + offsetMax },// max
                dash.image45
        );
        renderer::getRen().getUIPipeline().addImage(
                { dash.position.x - offsetMax, dash.position.y - offset },   // min
                { dash.position.x - offset, dash.position.y - offsetMax },   // max
                dash.image45down
        );
        renderer::getRen().getUIPipeline().addImage(
                { dash.position.x - offsetMax, dash.position.y + offset },   // min
                { dash.position.x - offset, dash.position.y + offsetMax },   // max
                dash.image45down
        );
    }
}

void rfct::gameplayButtonRenderInfo::bindImages() {
    // init images
    Images.reserve(9);
    Images.push_back(std::make_unique<bindableImage>("UI/left.png"));
    gameplayButtonBindings::buttonBindings.walkLeft.image = Images.back().get();
    Images.push_back(std::make_unique<bindableImage>("UI/right.png"));
    gameplayButtonBindings::buttonBindings.walkRight.image = Images.back().get();
    Images.push_back(std::make_unique<bindableImage>("UI/jump.png"));
    gameplayButtonBindings::buttonBindings.jump.image = Images.back().get();
    Images.push_back(std::make_unique<bindableImage>("UI/menu.png"));
    gameplayButtonBindings::buttonBindings.menu.image = Images.back().get();
    Images.push_back(std::make_unique<bindableImage>("UI/hold.png"));
    gameplayButtonBindings::buttonBindings.hold.image = Images.back().get();
    Images.push_back(std::make_unique<bindableImage>("UI/dash.png"));
    gameplayButtonBindings::buttonBindings.dash.image = Images.back().get();
    Images.push_back(std::make_unique<bindableImage>("UI/dash45.png"));
    gameplayButtonBindings::buttonBindings.dash.image45 = Images.back().get();
    Images.push_back(std::make_unique<bindableImage>("UI/dashUp.png"));
    gameplayButtonBindings::buttonBindings.dash.imageUp = Images.back().get();
    Images.push_back(std::make_unique<bindableImage>("UI/dash45down.png"));
    gameplayButtonBindings::buttonBindings.dash.image45down = Images.back().get();

}


