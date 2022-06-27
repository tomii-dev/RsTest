#include "app.hpp"

#include <iostream>
#include <d3/d3renderstream.h>

#include "scene.hpp"
#include "object.hpp"
#include "utils.hpp"
#include "control.hpp"

App* App::s_instance = nullptr;

App::App() : m_window		(nullptr), 
			 m_currentScene	(nullptr),
			 m_windowWidth	(800.f),
			 m_windowHeight	(600.f)
{
	s_instance = this;
    loadRenderStream();
}

App* App::getInstance() {
	return s_instance;
}

Scene* App::getScene() {
	return m_currentScene;
}

void App::setScene(Scene* scene) {
	m_currentScene = scene;
}

int App::run() {
	if (!glfwInit())
		return -1;

	m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "hellworld", NULL, NULL);
	if (!m_window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(m_window);
	glfwSwapInterval(1);

	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
	//	//glViewport(0, 0, App::getInstance()->getWindowWidth(), App::getInstance()->getWindowHeight());
	//	//App::getInstance()->setWindowWidth(width);
	//	//App::getInstance()->setWindowHeight(height);
	//});

	Control::setup();

	glewExperimental = GL_TRUE;
	glewInit();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	utils::setup();

	Scene scene(this);
	setScene(&scene);

	LightSource* light = scene.addLightSource(glm::vec3(-15.f, 15.f, 15.f), 1.f, .4f, VEC1);
	scene.addObject(ObjectType::Sphere, ObjectArgs{ VEC0, 1000.f, VEC1, "LINUS.jpg" });
	//sphere->rotate(92.f, glm::vec3(1, 0, 0));

	while (!glfwWindowShouldClose(m_window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Control::update();

		m_currentScene->update();
		m_currentScene->render();

		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}

	return 0;
}

float App::getWindowWidth() {
	return m_windowWidth;
}

float App::getWindowHeight() {
	return m_windowHeight;
}

GLFWwindow* App::getWindow() {
	return m_window;
}

void App::setWindowWidth(float width) {
	m_windowWidth = width;
	m_currentScene->updateMatrices();
}

void App::setWindowHeight(float height) {
	m_windowHeight = height;
	m_currentScene->updateMatrices();
}
