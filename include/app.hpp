#pragma once

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <windows.h>
#include <unordered_map>
#include <d3/d3renderstream.h>

#include "utils.hpp"

class Scene;

class App {
private:
	GLFWwindow* m_window;
	float m_windowWidth;
	float m_windowHeight;
	Scene* m_currentScene;
	static App* s_instance;
	HMODULE m_rsLib;
	TargetMap m_targets;
	FrameData m_frame;
	const StreamDescriptions* m_header;
    std::vector<uint8_t> m_desc;
    int loadRenderStream();
	int handleStreams();
	int sendFrames();
public:
	App();
	int run();
	float getWindowWidth();
	float getWindowHeight();
	void setWindowWidth(float width);
	void setWindowHeight(float height);
	static App* getInstance();
};