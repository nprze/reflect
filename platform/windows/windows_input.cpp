#include "input.h"
#include "renderer_p/renderer.h"
#include "key_bindings.h"
#include <glm/glm.hpp>

namespace rfct {
	GLFWwindow* window = nullptr;
	input inputInstance;
	input& input::getInput() { return inputInstance; }

	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (action == GLFW_PRESS) {
			input::getInput().anyClicked = true;
			input::getInput().clickExpiryTime = 0.2f;
		}
	}

	input::input() 
		:walk(0), 
		jump(0), 
		dashDefault(0), 
		dashX(0), 
		dash45up(0), 
		dash45down(0), 
		dashY(0), 
		hold(false), 
		windowExtent(nullptr) {
	}

	void input::init() {
		RFCT_PROFILE_FUNCTION();
		windowExtent = &(renderer::getRen().getWindow().extent);
		window = renderer::getRen().getWindow().GetHandle();
		glfwSetKeyCallback(window, key_callback);
	}

	void input::pollAndParseEvents(frameContext* context) {
		RFCT_PROFILE_FUNCTION();
		// reset
		hold = false;
		walk = 0;
		jump = 0;
		dashX = 0;
		dashY = 0;
		dash45up = 0;
		dash45down = 0;
		dashDefault = 0;
		upDown = 0;
		openClosePauseMenu = false;
		selectMenu = false;
		upDownMenu = 0;
		leftRightMenu = 0;

		// click is different from press
		if (anyClicked) {
			clickExpiryTime -= context->dt;
			if (clickExpiryTime <= 0) {
				anyClicked = false;
			}
		}

		// poll
		glfwPollEvents();

		// parse
		if (glfwGetKey(window, keyBindings::menu)) {
			openClosePauseMenu = true;
		}
		switch (context->state)
		{
		case gameState::gameplay:
		{
			if (glfwGetKey(window, keyBindings::hold) == GLFW_PRESS) {
				hold = true;
			}
			if (glfwGetKey(window, keyBindings::walk_right) == GLFW_PRESS) {
				walk += 1;
			}
			if (glfwGetKey(window, keyBindings::walk_left) == GLFW_PRESS) {
				walk -= 1;
			}
			if (glfwGetKey(window, keyBindings::jump) == GLFW_PRESS)
			{
				jump += 1;
			}
			if (glfwGetKey(window, keyBindings::dash) == GLFW_PRESS) {
				dashHelper.x = 0;
				dashHelper.y = 0;
				if (glfwGetKey(window, keyBindings::dash_dir_right) == GLFW_PRESS) {
					++dashHelper.x;
				}
				if (glfwGetKey(window, keyBindings::dash_dir_left) == GLFW_PRESS) {
					--dashHelper.x;
				}
				if (glfwGetKey(window, keyBindings::dash_dir_top) == GLFW_PRESS) {
					++dashHelper.y;
				}
				if (glfwGetKey(window, keyBindings::dash_dir_bottom) == GLFW_PRESS) {
					--dashHelper.y;
				}

				if (dashHelper.x != 0 && dashHelper.y == 0) { // horizontal
					dashX = dashHelper.x;
				}
				else if (dashHelper.y != 0 && dashHelper.x == 0) { // vertical
					dashY = dashHelper.y;
				}
				else if (dashHelper.x != 0 && dashHelper.y != 0) { // diagonal
					if (dashHelper.x > 0 && dashHelper.y > 0) {
						dash45up = 1;
					}
					else if (dashHelper.x > 0 && dashHelper.y < 0) {
						dash45down = 1;
					}
					else if (dashHelper.x < 0 && dashHelper.y > 0) {
						dash45down = -1;
					}
					else if (dashHelper.x < 0 && dashHelper.y < 0) {
						dash45up = -1;
					}
				}
				else { // default
					dashDefault = 1;
				}
			}
			if (glfwGetKey(window, keyBindings::dash_dir_top) == GLFW_PRESS) {
				upDown += 1;
			}
			if (glfwGetKey(window, keyBindings::dash_dir_bottom) == GLFW_PRESS) {
				upDown -= 1;
			}
			break;
		}
		case gameState::stateDialogue: {
			break;
		}
		case gameState::menu: {
			if (glfwGetKey(window, keyBindings::dash_dir_top) == GLFW_PRESS) {
				upDownMenu += 1;
			}
			if (glfwGetKey(window, keyBindings::dash_dir_bottom) == GLFW_PRESS) {
				upDownMenu -= 1;
			}
			if (glfwGetKey(window, keyBindings::dash_dir_right) == GLFW_PRESS) {
				leftRightMenu += 1;
			}
			if (glfwGetKey(window, keyBindings::dash_dir_left) == GLFW_PRESS) {
				leftRightMenu -= 1;
			}
			if (glfwGetKey(window, keyBindings::menu_select) == GLFW_PRESS) {
				selectMenu = true;
			}
			break;
		}
		default:
			break;
		}
	}
	button* input::addClickableButton(glm::vec2 pos, glm::vec2 size) {
		return nullptr;
	}
}