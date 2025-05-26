#include "input.h"
#include "renderer_p/renderer.h"
#include "key_bindings.h"
namespace rfct {
	input input::s_input;
	GLFWwindow* window = nullptr;
	input::input() :xAxis(0), yAxis(0), zAxis(0), cameraXAxis(0), cameraYAxis(0), cameraZAxis(0), windowExtent(nullptr)
	{
	}
	void input::init()
	{
		windowExtent = &(renderer::getRen().getWindow().extent);
		window = renderer::getRen().getWindow().GetHandle();
	}
	void input::pollAndParseEvents(frameContext* context) {
		glfwPollEvents();
		
		m_timeElapsedSinceStateChanged = std::clamp(m_timeElapsedSinceStateChanged + context->dt, 0.f, 1.f);
		if (glfwGetKey(window, key_menu) && m_timeElapsedSinceStateChanged > 0.5f) {
			if (m_previousState == gameState::gameplay) m_previousState = gameState::menu;
			else if (m_previousState == gameState::menu) m_previousState = gameState::gameplay;
		}
		context->state = m_previousState;

		xAxis=0;
		yAxis=0;
		zAxis=0;
		cameraXAxis=0;
		cameraYAxis=0;
		cameraZAxis=0;
		/*if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true);
		}*/
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && !(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)) { // Scene movement
			xAxis += 1;
		}
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)) //Camera movement
		{
			cameraXAxis += 1;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && !(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)) { // Scene movement
			xAxis -= 1;
		}
		else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)) //Camera movement
		{
			cameraXAxis -= 1;
		}
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)) //Camera movement
		{
			cameraYAxis -= 1;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)) //Camera movement
		{
			cameraYAxis += 1;
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) // Scene movement
		{
			yAxis += 1;
		}

	}
	button* input::addClickableButton(glm::vec2 pos, glm::vec2 size)
	{
		return nullptr;
	}
}