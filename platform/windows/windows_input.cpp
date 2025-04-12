#include "input.h"
#include "renderer_p\renderer.h"
namespace rfct {
	/*void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos); 
		mouseEvents.push(MouseEvent(MouseEvent::BUTTON_PRESS, xpos, ypos, button, action));
	}

	void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
		mouseEvents.push(MouseEvent(MouseEvent::MOVE, xpos, ypos));
	}

	void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
		mouseEvents.push(MouseEvent(MouseEvent::SCROLL, 0, 0, 0, 0, yoffset));
	}*/

	input* input::s_input;
	GLFWwindow* window = nullptr;
	input::input():xAxis(0), yAxis(0), zAxis(0), cameraXAxis(0), cameraYAxis(0), cameraZAxis(0), windowExtent(renderer::getRen().getWindow().extent)
	{
		window = renderer::getRen().getWindow().GetHandle();
		s_input = this;
	}
	void input::pollEvents() {
		glfwPollEvents();
		xAxis=0;
		yAxis=0;
		zAxis=0;
		cameraXAxis=0;
		cameraYAxis=0;
		cameraZAxis=0;
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true);
		}
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
}