#include "app.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL

#include <iostream>
#include <shlwapi.h>
#include <tchar.h>
#include <GLFW/glfw3native.h>
#include <d3/d3renderstream.h>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <fstream>
#include <cmath>

#include "scene.hpp"
#include "object.hpp"
#include "utils.hpp"

#pragma warning(disable:4996)

App* App::s_instance = nullptr;

App::App() : m_window		(nullptr),
             m_currentScene	(nullptr),
             m_rsLib		(nullptr),
             m_header		(nullptr),
             m_windowWidth	(1920.f),
             m_windowHeight	(1080.f),
             m_frame        (),
             m_schema       ()
{
	s_instance = this;
}

int App::loadRenderStream()
{
    HKEY key;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\d3 Technologies\\d3 Production Suite", 0, KEY_READ, &key)) 
        return utils::error("failed to open d3 registry key! do you have the disguise software installed?");
    TCHAR buf[512];
    DWORD bufSize = sizeof(buf);
    if (RegQueryValueEx(key, L"exe path", 0, nullptr, reinterpret_cast<LPBYTE>(buf), &bufSize))
        return utils::error("failed to query value of 'exe path' :(");
    if (!PathRemoveFileSpec(buf))
        return utils::error("failed to remove file spec from path :(");
    _tcscat_s(buf, bufSize, L"\\d3renderstream.dll");
    m_rsLib = LoadLibraryEx(buf, NULL,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR    | 
        LOAD_LIBRARY_SEARCH_APPLICATION_DIR | 
        LOAD_LIBRARY_SEARCH_SYSTEM32        | 
        LOAD_LIBRARY_SEARCH_USER_DIRS);

    if (m_rsLib == nullptr)
        return utils::error("failed to load renderstream dll!");

#define LOAD_FN(FUNC_NAME) \
    decltype(FUNC_NAME)* FUNC_NAME = reinterpret_cast<decltype(FUNC_NAME)>(GetProcAddress(m_rsLib, #FUNC_NAME)); \
    if (!FUNC_NAME) \
        std::wcerr << "Failed to get function " #FUNC_NAME " from DLL" << std::endl; \

    LOAD_FN(rs_initialise);
    LOAD_FN(rs_initialiseGpGpuWithOpenGlContexts);
    LOAD_FN(rs_logToD3);
    LOAD_FN(rs_getStreams);
    LOAD_FN(rs_sendFrame);
    LOAD_FN(rs_getFrameCamera);
    LOAD_FN(rs_awaitFrameData);
    LOAD_FN(rs_shutdown);
    LOAD_FN(rs_setSchema);
    LOAD_FN(rs_getFrameParameters);

    if (rs_initialise(RENDER_STREAM_VERSION_MAJOR, RENDER_STREAM_VERSION_MINOR))
        return utils::error("failed to init RenderStream!");

    utils::rsInitialiseGpuOpenGl	= rs_initialiseGpGpuWithOpenGlContexts;
    utils::logToD3					= rs_logToD3;
    utils::rsGetStreams				= rs_getStreams;
    utils::rsSendFrame				= rs_sendFrame;
    utils::rsGetFrameCamera			= rs_getFrameCamera;
    utils::rsAwaitFrameData			= rs_awaitFrameData;
    utils::rsShutdown				= rs_shutdown;
    utils::rsSetSchema              = rs_setSchema;
    utils::rsGetFrameParams         = rs_getFrameParameters;

    return 0;
}

int App::handleStreams() 
{
	RS_ERROR err = utils::rsAwaitFrameData(5000, &m_frame);
	switch (err) {
	case RS_ERROR_STREAMS_CHANGED:
		try {
			m_header = utils::getStreams(m_desc);
			size_t nStreams = m_header ? m_header->nStreams : 0;
			for (size_t i = 0; i < nStreams; ++i) {
				const StreamDescription& desc = m_header->streams[i];
				RenderTarget& target = m_targets[desc.handle];
				glGenTextures(1, &target.texture);
				utils::checkGLError(" generating tex");
				glBindTexture(GL_TEXTURE_2D, target.texture);
				utils::checkGLError(" binding texture");

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

				glTexImage2D(GL_TEXTURE_2D, 0, utils::glInternalFormat(desc.format), desc.width, desc.height, 
                                            0, utils::glFormat(desc.format), utils::glType(desc.format), nullptr);

				glBindTexture(GL_TEXTURE_2D, 0);

				glGenFramebuffers(1, &target.frameBuf);
				glBindFramebuffer(GL_FRAMEBUFFER, target.frameBuf);

				unsigned int rbo;
				glGenRenderbuffers(1, &rbo);
				glBindRenderbuffer(GL_RENDERBUFFER, rbo);
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, desc.width, desc.height);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target.texture, 0);

				GLenum bufs[] = { GL_COLOR_ATTACHMENT0 };
				glDrawBuffers(1, bufs);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
		}
        catch (const std::exception& e) {
            return utils::error(e.what());
        }
	case RS_ERROR_TIMEOUT:
	case RS_ERROR_SUCCESS: return 0;
    case RS_ERROR_QUIT: return 0;
	default:
        return utils::error("rs_awaitFrameData returned " + utils::rsErrorStr(err));
	}
}

int App::sendFrames() 
{
    const size_t nStreams = m_header ? m_header->nStreams : 0;
    for (size_t i = 0; i < nStreams; ++i) {
        const StreamDescription& desc = m_header->streams[i];
        CameraResponseData res;
        res.tTracked = m_frame.tTracked;
        if (utils::rsGetFrameCamera(desc.handle, &res.camera) == RS_ERROR_SUCCESS) {
            const RenderTarget& target = m_targets.at(desc.handle);
            setWindowWidth(desc.width);
            setWindowHeight(desc.height);
            glViewport(0, 0, desc.width, desc.height);
            glBindFramebuffer(GL_FRAMEBUFFER, target.frameBuf);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            const RemoteParameters& rsScene = m_schema.scenes.scenes[m_frame.scene];
            std::vector<float> params(rsScene.nParameters);
            if (utils::rsGetFrameParams(rsScene.hash, params.data(), params.size() * sizeof(float)))
                return utils::error("failed to get frame params");
            m_currentScene = m_scenes[m_frame.scene];
            if (m_updateQueue.objects.size())
            {
                for (const ObjectConfig& obj : m_updateQueue.objects)
                    m_currentScene->addObject(obj.type, obj.args);
                m_updateQueue.clear();
            }
            Camera* cam = m_currentScene->getCurrentCamera();
            cam->setPosition(glm::vec3(res.camera.z, -res.camera.y, res.camera.x));
            cam->setRotation(res.camera.rz, res.camera.ry, res.camera.rx);
            m_currentScene->update();
            m_currentScene->render();
            SenderFrameTypeData data;
            data.gl.texture = target.texture;
            if (utils::rsSendFrame(desc.handle, RS_FRAMETYPE_OPENGL_TEXTURE, data, &res))
			    return 1;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }
    return 0;
}

void App::measureFps()
{
    double currentTime = glfwGetTime();
    m_frameInfo.frameCount++;
    if (currentTime - m_frameInfo.previousTime >= 1.0)
    {
        m_metrics.fps = m_frameInfo.frameCount;

        m_frameInfo.frameCount = 0;
        m_frameInfo.previousTime = currentTime;
    }
}

void App::renderUi()
{
    // switch context to window for ui rendering
    glfwMakeContextCurrent(m_uiWindow);

    glClear(GL_COLOR_BUFFER_BIT);

    // start new imgui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // each imgui window needs to take up half of os window
    // on y axis
    const double winX = ImGui::GetIO().DisplaySize.x;
    const double winHalfY = ImGui::GetIO().DisplaySize.y / 2;
    ImGui::SetNextWindowSize(ImVec2(winX, winHalfY));
    ImGui::SetNextWindowPos(ImVec2());

    // we don't want the imgui windows to be resized or moved
    const int flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    ImGui::Begin("Metrics", 0, flags);
    ImGui::LabelText(std::to_string(m_metrics.fps).c_str(), "FPS");
    ImGui::End();
    ImGui::SetNextWindowSize(ImVec2(winX, winHalfY));
    ImGui::SetNextWindowPos(ImVec2(0, winHalfY));
    ImGui::Begin("Controls", 0, flags);
    ImGui::Combo("Colour Space", (int*) &m_config.colourSpace, colourSpaces, IM_ARRAYSIZE(colourSpaces));

    if (ImGui::Button("Add object"))
        m_uiState.addObjectWinOpen = true;

    if (m_uiState.addObjectWinOpen)
    {
        ObjectConfig* obj = &m_uiState.currentObj;
        ImGui::SetNextWindowSize(ImVec2(275, 100));
        ImGui::Begin("Add object", 0, ImGuiWindowFlags_NoResize);
        ImGui::Combo("Object type", reinterpret_cast<int*>(&obj->type), objectTypes, IM_ARRAYSIZE(objectTypes));
        ImGui::InputFloat3("Position", &obj->args.pos[0]);
        if (ImGui::Button("Add"))
        {
            m_uiState.addObjectWinOpen = false;
            m_updateQueue.objects.push_back(*obj);
        }
        ImGui::End();
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_uiWindow);
}

App* App::getInstance() 
{
	return s_instance;
}

RsSchema& App::getSchema()
{
    return s_instance->m_schema;
}

const Params& App::getParams()
{
    return s_instance->m_params;
}

int App::run() 
{
    if (loadRenderStream())
        return 1;

    // initialise glfw lib
    if (!glfwInit())
        return utils::error("failed to initialise GLFW!");

    // create window and return if failed
	m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "RsTest", NULL, NULL);
    if (!m_window)
        utils::error("failed to create window :(");

    // create window for metrics and controls
    m_uiWindow = glfwCreateWindow(300, 200, "RsTest", NULL, NULL);
    if (!m_uiWindow)
        utils::error("failed to create ui window :(");
    glfwSetWindowSizeLimits(m_uiWindow, 300, 200, 600, 400);
    GLFWimage img;
    img.pixels = stbi_load("C:/Program Files/RsTest/img/icon.png", &img.width, &img.height, 0, 4);
    glfwSetWindowIcon(m_uiWindow, 1, &img);
    stbi_image_free(img.pixels);

    // hide window and set it to be current opengl context
	glfwHideWindow(m_window);
	glfwMakeContextCurrent(m_window);

    // set up imgui for metrics window
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(m_uiWindow, true);
    ImGui_ImplOpenGL3_Init("#version 120");
    ImGui::StyleColorsDark();
    ImGui::SetNextWindowSize(ImVec2(300, 200));

    // initialise glew library, used to get openGL functions
	glewExperimental = GL_TRUE;
	glewInit();

    // enable gl depth testing and set to draw when the incoming depth value is less than the stored depth value
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	HGLRC wglContext = glfwGetWGLContext(m_window);
	HDC dc = GetDC(glfwGetWin32Window(m_window));

    m_scenes.push_back(new Scene("scene 1"));
    m_scenes.push_back(new Scene("scene 2"));
    m_currentScene = m_scenes[0];
	LightSource* light = m_currentScene->addLightSource(glm::vec3(20.f, -15.f, 0.f), 1.f, .4f, VEC1);

	if(utils::rsInitialiseGpuOpenGl(wglContext, dc))
        utils::error("failed to initialise RenderStream GPU interop");

    if (utils::rsSetSchema(&m_schema))
        return utils::error("failed to set schema!");
   
    m_frameInfo = FrameInfo(glfwGetTime());

	while(true)
	{
        glfwMakeContextCurrent(m_window);

        measureFps();

		if(handleStreams())
			break;

		if (sendFrames())
			break;

        renderUi();

		glfwPollEvents();
	}

	return utils::rsShutdown();
}

float App::getWindowWidth() 
{
	return m_windowWidth;
}

float App::getWindowHeight() 
{
	return m_windowHeight;
}

void App::setWindowWidth(float width) {
	m_windowWidth = width;
	m_currentScene->updateMatrices();
}

void App::setWindowHeight(float height) {
	m_windowHeight = height;
	m_currentScene->updateMatrices();
}
