#include "app.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL

#include <iostream>
#include <shlwapi.h>
#include <tchar.h>
#include <GLFW/glfw3native.h>
#include <d3/d3renderstream.h>

#include "scene.hpp"
#include "object.hpp"
#include "utils.hpp"

App* App::s_instance = nullptr;
decltype(rs_logToD3)* g_log;

App::App() : m_window		(nullptr),
			 m_currentScene	(nullptr),
			 m_rsLib		(nullptr),
			 m_header		(nullptr),
			 m_windowWidth	(1920.f),
			 m_windowHeight	(1080.f)
{
	s_instance = this;
    loadRenderStream();
}

void App::loadRenderStream() {
    HKEY key;
    RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\d3 Technologies\\d3 Production Suite"), 0, KEY_READ, &key);
    TCHAR buf[512];
    DWORD bufSize = sizeof(buf);
    RegQueryValueEx(key, TEXT("exe path"), 0, nullptr, reinterpret_cast<LPBYTE>(buf), &bufSize);
    PathRemoveFileSpec(buf);
    _tcscat_s(buf, bufSize, TEXT("\\d3renderstream.dll"));
    m_rsLib = LoadLibraryEx(buf, NULL,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR    | 
        LOAD_LIBRARY_SEARCH_APPLICATION_DIR | 
        LOAD_LIBRARY_SEARCH_SYSTEM32        | 
        LOAD_LIBRARY_SEARCH_USER_DIRS);

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

	rs_initialise(RENDER_STREAM_VERSION_MAJOR, RENDER_STREAM_VERSION_MINOR);

	utils::rsInitialiseGpuOpenGl	= rs_initialiseGpGpuWithOpenGlContexts;
	utils::logToD3					= rs_logToD3;
	utils::rsGetStreams				= rs_getStreams;
	utils::rsSendFrame				= rs_sendFrame;
	utils::rsGetFrameCamera			= rs_getFrameCamera;
	utils::rsAwaitFrameData			= rs_awaitFrameData;
	utils::rsShutdown				= rs_shutdown;
}

int App::handleStreams() {
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

				glTexImage2D(GL_TEXTURE_2D, 0, utils::glInternalFormat(desc.format), desc.width, desc.height, 0, utils::glFormat(desc.format), utils::glType(desc.format), nullptr);

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
			utils::logToD3(e.what());
			return 1;
		}
	case RS_ERROR_TIMEOUT:
	case RS_ERROR_SUCCESS: return 0;
	default:
		utils::logToD3("rs_awaitFrameData returned " + err);
		return 1;
	}
}

int App::sendFrames() {
	const size_t nStreams = m_header ? m_header->nStreams : 0;
	for (size_t i = 0; i < nStreams; ++i) {
		const StreamDescription& desc = m_header->streams[i];
		CameraResponseData res;
		res.tTracked = m_frame.tTracked;
		if (utils::rsGetFrameCamera(desc.handle, &res.camera) == RS_ERROR_SUCCESS) {
			const RenderTarget& target = m_targets.at(desc.handle);
			glBindFramebuffer(GL_FRAMEBUFFER, target.frameBuf);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			m_currentScene->update();
			m_currentScene->render();
			glFinish();
			SenderFrameTypeData data;
			data.gl.texture = target.texture;
			if (utils::rsSendFrame(desc.handle, RS_FRAMETYPE_OPENGL_TEXTURE, data, &res))
				return 1;
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			m_currentScene->update();
			m_currentScene->render();
			glfwSwapBuffers(m_window);
		}
	}
	return 0;
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

	m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "RsTest", NULL, NULL);
	if (!m_window) {
		glfwTerminate();
		return -1;
	}

	glfwHideWindow(m_window);
	glfwMakeContextCurrent(m_window);

	glewExperimental = GL_TRUE;
	glewInit();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	HGLRC wglContext = glfwGetWGLContext(m_window);
	HDC dc = GetDC(glfwGetWin32Window(m_window));

	Scene scene(this);
	setScene(&scene);

	LightSource* light = scene.addLightSource(glm::vec3(0, -5.f, 0), 1.f, .4f, VEC1);
	Object* sphere = scene.addObject(ObjectType::Sphere, ObjectArgs{ VEC0, 1.f, glm::vec3(0, .7f, 1.f)});

	utils::rsInitialiseGpuOpenGl(wglContext, dc);

	while (true) {

		if(handleStreams())
			break;

		if (sendFrames())
			break;

		glfwPollEvents();
	}

	return utils::rsShutdown();
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
