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

struct ObjectConfig
{
    ObjectType type;
    ObjectArgs args;
};

struct SceneConfig
{
    std::string name;
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

struct UiState
{
    bool addObjectWinOpen;
    bool remObjectWinOpen;
    bool newSceneWinOpen;
    ObjectConfig currentAddObj;
    int currentRemObj;
    SceneConfig currentScene;
    bool exit;
};

struct UpdateQueue
{
    ObjectConfig* addObject;
    Object* removeObject;
    SceneConfig* addScene;
    void clear()
    {
        // do not deallocate the object being removed as this has to be passed to
        // Scene::removeObject and will then be deallocated
        delete addObject;
        delete addScene;

        addObject = nullptr;
        removeObject = nullptr;
        addScene = nullptr;
    }
    bool empty()
    {
        return !addObject && !removeObject && !addScene;
    }
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
    std::vector<Scene*> m_scenes;
    Scene* m_currentScene;
    static App* s_instance;
    HMODULE m_rsLib;
    TargetMap m_targets;
    FrameData m_frame;
    RsSchema m_schema;
    const StreamDescriptions* m_header;
    UiState m_uiState;
    UpdateQueue m_updateQueue;
    std::vector<uint8_t> m_desc;
    std::vector<float> m_params;
    std::vector<ImageFrameData> m_imgData;
    uint64_t m_hash;
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
    static const std::vector<float>& getParams();
    static const std::vector<ImageFrameData>& getImgData();
    static Scene* getCurrentScene();
    static void reloadSchema();
};