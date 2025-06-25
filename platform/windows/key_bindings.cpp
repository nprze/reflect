#include "key_bindings.h"
#define DEFAULT_KEY_SETUP 1
namespace rfct {
#if DEFAULT_KEY_SETUP == 0
	int keyBindings::menu = GLFW_KEY_ESCAPE;
	int keyBindings::walk_right = GLFW_KEY_D;
	int keyBindings::walk_left = GLFW_KEY_A;
	int keyBindings::jump = GLFW_KEY_SPACE;
	int keyBindings::dash = GLFW_KEY_LEFT_SHIFT;
	int keyBindings::dash_dir_right = GLFW_KEY_D;
	int keyBindings::dash_dir_left = GLFW_KEY_A;
	int keyBindings::dash_dir_top = GLFW_KEY_W;
	int keyBindings::dash_dir_bottom = GLFW_KEY_S;
#else
	int keyBindings::menu = GLFW_KEY_ESCAPE;
	int keyBindings::walk_right = GLFW_KEY_RIGHT;
	int keyBindings::walk_left = GLFW_KEY_LEFT;
	int keyBindings::jump = GLFW_KEY_SPACE;
	int keyBindings::dash = GLFW_KEY_LEFT_SHIFT;
	int keyBindings::dash_dir_right = GLFW_KEY_RIGHT;
	int keyBindings::dash_dir_left = GLFW_KEY_LEFT;
	int keyBindings::dash_dir_top = GLFW_KEY_UP;
	int keyBindings::dash_dir_bottom = GLFW_KEY_DOWN;
#endif
}