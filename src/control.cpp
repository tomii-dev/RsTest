#include "control.hpp"

#include "app.hpp"
#include "camera.hpp"
#include "scene.hpp"

std::map<int, bool> Control::s_keyMap;
glm::vec2 Control::s_lastMouse = glm::vec2(0, 0);

void Control::setup() {
	GLFWwindow* window = App::getInstance()->getWindow();
	glfwSetKeyCallback(window, handleKey);
	glfwSetCursorPosCallback(window, handleMouse);
}

void Control::update() {
	for (const auto& kv : s_keyMap) {
		if (!kv.second)
			continue;
		Camera* cam = App::getInstance()->getScene()->getCurrentCamera();
		switch (kv.first) {
		case GLFW_KEY_W:
			cam->moveForward();
			break;
		case GLFW_KEY_S:
			cam->moveBackward();
			break;
		case GLFW_KEY_A:
			cam->moveLeft();
			break;
		case GLFW_KEY_D:
			cam->moveRight();
			break;
		case GLFW_KEY_SPACE:
			cam->moveUp();
			break;
		case GLFW_KEY_LEFT_CONTROL:
			cam->moveDown();
			break;
		}
	}
}

void Control::handleKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
	s_keyMap[key] = (bool)action;
}

void Control::handleMouse(GLFWwindow* window, double x, double y) {
	glm::vec2 offset(
		x - s_lastMouse.x,
		s_lastMouse.y - y
	);
	s_lastMouse = glm::vec2(x, y);
	if (offset.y > 89.f)
		offset.y = 89.f;
	if (offset.y < -89.f)
		offset.y = -89.f;
	App::getInstance()->getScene()->getCurrentCamera()
		->rotate(offset * 0.1f);
}