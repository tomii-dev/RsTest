#pragma once

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <windows.h>

class Scene;

class App {
	GLFWwindow* m_window;
	float m_windowWidth;
	float m_windowHeight;
	Scene* m_currentScene;
    HMODULE m_rsLib;
	static App* s_instance;
	void setScene(Scene* scene);
    void loadRenderStream();
public:
	App();
	int run();
	float getWindowWidth();
	float getWindowHeight();
	GLFWwindow* getWindow();
	Scene* getScene();
	void setWindowWidth(float width);
	void setWindowHeight(float height);
	static App* getInstance();
};