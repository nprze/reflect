#include "button_bindings.h"
#include "renderer_p/renderer.h"
#include <glm/trigonometric.hpp>
#include <glm/gtc/constants.hpp>
#include "assets/assets_manager.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

constexpr float timeForDashAfterBasicButtonReleased = 0.3f;
rfct::gameplayButtonBindings rfct::gameplayButtonBindings::buttonBindings;
rfct::gameplayButtonRenderInfo rfct::gameplayButtonRenderInfo::buttonRenderInfo;

void rfct::gameplayButtonBindings::clickCheck(const glm::vec2 &point, const int& action, const int& pointerID, const frameContext* ctx) {
	RFCT_PROFILE_FUNCTION();
    dashBttn.updateIsClicked(point, action, pointerID);
    jump.updateIsClicked(point, action, pointerID);
    menu.updateIsClicked(point, action, pointerID);
    hold.updateIsClicked(point, action, pointerID);
    movement.update(point, action, pointerID, ctx->dt);
}

void rfct::gameplayButtonBindings::updateInput(rfct::input &input) const {
    RFCT_PROFILE_FUNCTION();
    input.walk = 0;
    input.jump = 0;
    input.openMenu = false;
    input.dashDefault = 0;
    input.dashX = 0;
    input.dash45up = 0;
    input.dash45down = 0;
    input.dashY = 0;
    input.hold = false;


    if (jump.isClicked){
        input.jump += 1;
    }
    if (menu.isClicked){
        input.openMenu = true;
    }
    if (hold.isClicked) {
        input.hold = true;
    }
    if (movement.moveDirection != dir::DirUndefined) {
        input.walk = (movement.moveDirection == dir::right) ? 1 : -1;
    }
    if (dashBttn.isClicked) {
        if (movement.dashDirection != dir::DirUndefined) {
            switch (movement.dashDirection) {
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
        else {
            input.dashDefault = 1;
        }
    }
}

void rfct::gameplayButtonBindings::init() {
    RFCT_PROFILE_FUNCTION();
    vk::Extent2D windowExtent = renderer::getRen().getWindow().getExtent();
    // walk bttns size
    glm::vec2 joystickSize = {((float)windowExtent.height) * 0.45f, ((float)windowExtent.height) * 0.45f };
    glm::vec2 smallerButtonSize = {((float)windowExtent.height) * 0.15f, ((float)windowExtent.height) * 0.15f };
    // basic layout:
    float tenPercentHeight = ((float)windowExtent.height) * 0.10f;
    // jump: 10% height from bottom, 30% height from right
    jump.minViewport = {((float)windowExtent.width) - smallerButtonSize.x - 3 * tenPercentHeight, ((float)windowExtent.height) - smallerButtonSize.y - tenPercentHeight};
    jump.maxViewport = jump.minViewport + smallerButtonSize;
    // hold: 30% height from bottom, 20% height from right
    hold.minViewport = { ((float)windowExtent.width) - smallerButtonSize.x - 2 * tenPercentHeight, ((float)windowExtent.height) - smallerButtonSize.y - 3 * tenPercentHeight };
    hold.maxViewport = hold.minViewport + smallerButtonSize;
    
    // hold: 50% height from bottom, 20% height from right
    dashBttn.minViewport = { ((float)windowExtent.width) - smallerButtonSize.x - 2 * tenPercentHeight, ((float)windowExtent.height) - smallerButtonSize.y - 5 * tenPercentHeight };
    dashBttn.maxViewport = dashBttn.minViewport + smallerButtonSize;

    // menu aligned to right top corner, width and height 10% of window height
    menu.minViewport = {windowExtent.width - tenPercentHeight, 0};
    menu.maxViewport = menu.minViewport + glm::vec2(tenPercentHeight, tenPercentHeight);

    // joystick middle: 15% height from bottom, 30% height from right
    movement.position = { 3.f * tenPercentHeight, ((float)windowExtent.height) - smallerButtonSize.y - 1.5f * tenPercentHeight };
    // joystick interaction radius: 25% height
    movement.intractionRadius = 3.0 * tenPercentHeight;
    // joystick draw radius: 20% height
    movement.drawRadius = 2.5f * tenPercentHeight;

    movement.joystickMiddleHalfSize = smallerButtonSize * 0.5f;
    movement.lastTouchPos = movement.position;

    gameplayButtonRenderInfo::buttonRenderInfo.bindImages("UI/android_buttons.txt");
}

void rfct::gameplayButtonBindings::drawButtons() {
    RFCT_PROFILE_FUNCTION();
    renderer::getRen().getUIPipeline().addImage(dashBttn.minViewport, dashBttn.maxViewport, gameplayButtonRenderInfo::buttonRenderInfo.Image.get(),            dashBttn.minImageReleased, dashBttn.maxImageReleased);
    renderer::getRen().getUIPipeline().addImage(jump.minViewport, jump.maxViewport, gameplayButtonRenderInfo::buttonRenderInfo.Image.get(),                    jump.minImageReleased, jump.maxImageReleased);
    renderer::getRen().getUIPipeline().addImage(menu.minViewport, menu.maxViewport, gameplayButtonRenderInfo::buttonRenderInfo.Image.get(),                    menu.minImageReleased, menu.maxImageReleased);
    renderer::getRen().getUIPipeline().addImage(hold.minViewport, hold.maxViewport, gameplayButtonRenderInfo::buttonRenderInfo.Image.get(),                    hold.minImageReleased, hold.maxImageReleased);

    renderer::getRen().getUIPipeline().addImage(movement.position - glm::vec2{movement.drawRadius, movement.drawRadius}, movement.position + glm::vec2{movement.drawRadius, movement.drawRadius}, gameplayButtonRenderInfo::buttonRenderInfo.JoystickImage.get());
    renderer::getRen().getUIPipeline().addImage(movement.lastTouchPos - movement.joystickMiddleHalfSize, movement.lastTouchPos + movement.joystickMiddleHalfSize, gameplayButtonRenderInfo::buttonRenderInfo.Image.get(), movement.joystickMiddleImageMin, movement.joystickMiddleImageMax);
}

void rfct::gameplayButtonRenderInfo::bindImages(const std::string& path) {
    RFCT_PROFILE_FUNCTION();
    buttonImageSerializeData serializeData;
    AssetsManager::get().loadButtonImage(path, &serializeData);
    // init images
    Image = std::make_unique<bindableImage>(serializeData.imagePath);
    JoystickImage = std::make_unique<bindableImage>(serializeData.joystickImagePath);

    float oneOverRowCount = 1.f / ((float)serializeData.imageRows);
    float oneOverColumnCount = 1.f / ((float)serializeData.imageColumns);

    gameplayButtonBindings::buttonBindings.movement.joystickMiddleImageMin = { serializeData.joystick.released.y * oneOverColumnCount + 0.003f, serializeData.joystick.released.x * oneOverRowCount + 0.003f };
    gameplayButtonBindings::buttonBindings.movement.joystickMiddleImageMax = { (serializeData.joystick.released.y +1) * oneOverColumnCount - 0.003f , (serializeData.joystick.released.x + 1) * oneOverRowCount -0.003f };

    gameplayButtonBindings::buttonBindings.dashBttn.minImageReleased = { serializeData.dash.released.y * oneOverColumnCount, serializeData.dash.released.x * oneOverRowCount };
    gameplayButtonBindings::buttonBindings.dashBttn.maxImageReleased = { (serializeData.dash.released.y +1) * oneOverColumnCount , (serializeData.dash.released.x + 1) * oneOverRowCount  };

    gameplayButtonBindings::buttonBindings.jump.minImageReleased = { serializeData.jump.released.y * oneOverColumnCount, serializeData.jump.released.x * oneOverRowCount };
    gameplayButtonBindings::buttonBindings.jump.maxImageReleased = { (serializeData.jump.released.y +1) * oneOverColumnCount , (serializeData.jump.released.x + 1) * oneOverRowCount  };

    gameplayButtonBindings::buttonBindings.hold.minImageReleased = { serializeData.hold.released.y * oneOverColumnCount, serializeData.hold.released.x * oneOverRowCount };
    gameplayButtonBindings::buttonBindings.hold.maxImageReleased = { (serializeData.hold.released.y +1) * oneOverColumnCount , (serializeData.hold.released.x + 1) * oneOverRowCount  };

    gameplayButtonBindings::buttonBindings.menu.minImageReleased = { serializeData.menu.released.y * oneOverColumnCount, serializeData.menu.released.x * oneOverRowCount  };
    gameplayButtonBindings::buttonBindings.menu.maxImageReleased = { (serializeData.menu.released.y +1) * oneOverColumnCount, (serializeData.menu.released.x + 1) * oneOverRowCount };
}

void rfct::joystick::update(const glm::vec2& point, const int& action, const int& pointer, const float& dt)
{
    bool inside = (glm::distance(point, position) < intractionRadius);
        
    if (action == 0) {
        if (inside) {
            activePointers.insert(pointer);
            lastTouchPos = point;
        }
    }
    else if (action == 1) {
        activePointers.erase(pointer);
        lastTouchPos = position;
    }
    else if (action == 2) {
        if (activePointers.count(pointer)) {
            if (!inside) {
                activePointers.erase(pointer);
                lastTouchPos = position;
            }
            else {
                lastTouchPos = point;
            }
        }
        else if (inside) {
            activePointers.insert(pointer);
            lastTouchPos = point;
        }
    }

    isTriggered = !activePointers.empty();
}

void rfct::joystick::updateDir()
{
    glm::vec2 direction = lastTouchPos - position;
    if (std::abs(direction.x) > 0.1 * intractionRadius) {
        if (direction.x > 0) {
            moveDirection = right;
        }
        else {
            moveDirection = left;
        }
    }
    else {
        moveDirection = dir::DirUndefined;
    }
    if (gameplayButtonBindings::buttonBindings.dashBttn.isClicked) {
        if (glm::length(direction) > 0.1 * intractionRadius) {
            direction = glm::normalize(lastTouchPos - position);
            float angle = atan2(direction.y, direction.x);


            dir directionEnum = DirUndefined;

            if (angle >= -M_PI / 8 && angle < M_PI / 8) {
                dashDirection = right;
            }
            else if (angle >= M_PI / 8 && angle < 3 * M_PI / 8) {
                dashDirection = rightBottom;
            }
            else if (angle >= 3 * M_PI / 8 && angle < 5 * M_PI / 8) {
                dashDirection = bottom;
            }
            else if (angle >= 5 * M_PI / 8 && angle < 7 * M_PI / 8) {
                dashDirection = leftBottom;
            }
            else if (angle >= 7 * M_PI / 8 || angle < -7 * M_PI / 8) {
                dashDirection = left;
            }
            else if (angle >= -7 * M_PI / 8 && angle < -5 * M_PI / 8) {
                dashDirection = leftTop;
            }
            else if (angle >= -5 * M_PI / 8 && angle < -3 * M_PI / 8) {
                dashDirection = top;
            }
            else if (angle >= -3 * M_PI / 8 && angle < -M_PI / 8) {
                dashDirection = rightTop;
            }
        }
        else {
            dashDirection = dir::DirUndefined;
        }
    }
    else {
        dashDirection = dir::DirUndefined;
    }
}
