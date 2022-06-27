#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <map>

class Control {
	static std::map<int, bool> s_keyMap;
	static glm::vec2 s_lastMouse;
public:
	static void setup();
	static void update();
	static void handleKey(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void handleMouse(GLFWwindow* window, double x, double y);
};