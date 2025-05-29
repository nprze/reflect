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
		walk = 0;
		jump = 0;

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

	}
	button* input::addClickableButton(glm::vec2 pos, glm::vec2 size)
	{
		return nullptr;
	}
}