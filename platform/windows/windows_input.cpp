#include "input.h"
#include "renderer_p/renderer.h"
#include "key_bindings.h"
#include <glm/glm.hpp>
#include "input.h"
namespace rfct {
	input input::s_input;
	GLFWwindow* window = nullptr;
	input::input() :walk(0), windowExtent(nullptr)
	{
	}
	void input::init()
	{
		windowExtent = &(renderer::getRen().getWindow().extent);
		window = renderer::getRen().getWindow().GetHandle();
	}
	void input::pollAndParseEvents(frameContext* context) {
		// reset
		hold = false;
		walk = 0;
		jump = 0;

		dashX = 0;
		dashY = 0;
		dash45up = 0;
		dash45down = 0;
		dashDefault = 0;
		
	

		glfwPollEvents();
		
		m_timeElapsedSinceStateChanged = std::clamp(m_timeElapsedSinceStateChanged + context->dt, 0.f, 1.f);
		if (glfwGetKey(window, keyBindings::menu) && m_timeElapsedSinceStateChanged > 0.5f) {
			if (m_previousState == gameState::gameplay) m_previousState = gameState::menu;
			else if (m_previousState == gameState::menu) m_previousState = gameState::gameplay;
		}
		context->state = m_previousState;

		/*if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true);
		}*/
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
				else if (dashHelper.x< 0 && dashHelper.y < 0) {
					dash45up = -1;
				}
			}
			else { // default
				dashDefault = 1;
			}
		}

	}
	button* input::addClickableButton(glm::vec2 pos, glm::vec2 size)
	{
		return nullptr;
	}
}