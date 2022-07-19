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

struct Params
{
    float sphereX = 0;
};

struct FrameInfo
{
    double previousTime;
    int frameCount = 0;
    FrameInfo() {}
    FrameInfo(double time) : previousTime(time) {}
};

struct Metrics
{
    float fps;
};

// struct to store state of controls in ui window
struct Config
{
    ColourSpace colourSpace;
};

class App {
private:
	GLFWwindow* m_window;
    GLFWwindow* m_uiWindow;
    Metrics m_metrics;
    Config m_config;
    FrameInfo m_frameInfo;
	float m_windowWidth;
	float m_windowHeight;
	Scene* m_currentScene;
	static App* s_instance;
	HMODULE m_rsLib;
	TargetMap m_targets;
	FrameData m_frame;
    RsSchema m_schema;
    Params m_params;
	const StreamDescriptions* m_header;
    std::vector<uint8_t> m_desc;
    int loadRenderStream();
	int handleStreams();
	int sendFrames();
    void measureFps();
    void renderUi();
public:
	App();
	int run();
	float getWindowWidth();
	float getWindowHeight();
	void setWindowWidth(float width);
	void setWindowHeight(float height);
	static App* getInstance();
    static RsSchema& getSchema();
    static const Params& getParams();
};