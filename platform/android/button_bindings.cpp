#include "button_bindings.h"
#include "renderer_p/renderer.h"

rfct::gameplayButtonBindings rfct::gameplayButtonBindings::buttonBindings;
rfct::gameplayButtonRenderInfo rfct::gameplayButtonRenderInfo::buttonRenderInfo;



void rfct::gameplayButtonBindings::startEventParse() {
    //walkRight.isClicked = false;
    //walkLeft.isClicked = false;
    //jump.isClicked = false;
    //menu.isClicked = false;
}

void rfct::gameplayButtonBindings::clickCheck(const glm::vec2 &point, const int& action, const int& pointerID){
    walkRight.updateIsClicked(point, action, pointerID);
    walkLeft.updateIsClicked(point, action, pointerID);
    jump.updateIsClicked(point, action, pointerID);
    menu.updateIsClicked(point, action, pointerID);
}

void rfct::gameplayButtonBindings::updateInput(rfct::input &input) const{
    input.walk = 0;
    input.jump = 0;
    input.openMenu = false;


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
}

void rfct::gameplayButtonBindings::init() {

    vk::Extent2D windowExtent = renderer::getRen().getWindow().getExtent();
    // walk bttns size
    glm::vec2 walkBttnsSize = {((float)windowExtent.height) * 0.25f, ((float)windowExtent.height) * 0.25f };
    // basic layout:
    // left: 10% height from bottom, 10% height from left
    float tenPercentHeight = ((float)windowExtent.height) * 0.10f;
    walkLeft.min = {tenPercentHeight, ((float)windowExtent.height) - walkBttnsSize.y - tenPercentHeight};
    walkLeft.max = walkLeft.min + walkBttnsSize;
    // right: 10% height from walk left, same height as walk left
    walkRight.min = {walkLeft.min.x + walkBttnsSize.x + tenPercentHeight, walkLeft.min.y};
    walkRight.max = walkRight.min + walkBttnsSize;
    // jump: 10% height from bottom, 10% height from right
    jump.min = {((float)windowExtent.width) - walkBttnsSize.x - tenPercentHeight, ((float)windowExtent.height) - walkBttnsSize.y - tenPercentHeight};
    jump.max = jump.min + walkBttnsSize;
    // menu aligned to right top corner, width and height 10% of window height
    menu.min = {windowExtent.width - tenPercentHeight, 0};
    menu.max = menu.min + glm::vec2(tenPercentHeight, tenPercentHeight);

    gameplayButtonRenderInfo::buttonRenderInfo.bindImages();
}

void rfct::gameplayButtonBindings::drawButtons() {
    renderer::getRen().getUIPipeline().addImage(walkLeft.min, walkLeft.max, walkLeft.image);
    renderer::getRen().getUIPipeline().addImage(walkRight.min, walkRight.max, walkRight.image);
    renderer::getRen().getUIPipeline().addImage(jump.min, jump.max, jump.image);
    renderer::getRen().getUIPipeline().addImage(menu.min, menu.max, menu.image);
}

void rfct::gameplayButtonRenderInfo::bindImages() {
    // init images
    Images.reserve(4);
    Images.push_back(std::make_unique<bindableImage>("UI/left.png"));
    gameplayButtonBindings::buttonBindings.walkLeft.image = Images.back().get();
    Images.push_back(std::make_unique<bindableImage>("UI/right.png"));
    gameplayButtonBindings::buttonBindings.walkRight.image = Images.back().get();
    Images.push_back(std::make_unique<bindableImage>("UI/jump.png"));
    gameplayButtonBindings::buttonBindings.jump.image = Images.back().get();
    Images.push_back(std::make_unique<bindableImage>("UI/menu.png"));
    gameplayButtonBindings::buttonBindings.menu.image = Images.back().get();
}

