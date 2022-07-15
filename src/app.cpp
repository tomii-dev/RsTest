#include "app.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL

#include <iostream>
#include <shlwapi.h>
#include <tchar.h>
#include <GLFW/glfw3native.h>
#include <d3/d3renderstream.h>
#include <glm/glm.hpp>

#include "scene.hpp"
#include "object.hpp"
#include "utils.hpp"

App* App::s_instance = nullptr;

App::App() : m_window		(nullptr),
             m_currentScene	(nullptr),
             m_rsLib		(nullptr),
             m_header		(nullptr),
             m_windowWidth	(1920.f),
             m_windowHeight	(1080.f),
             m_frame        ()
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

    if (rs_initialise(RENDER_STREAM_VERSION_MAJOR, RENDER_STREAM_VERSION_MINOR))
        return utils::error("failed to init RenderStream!");

    utils::rsInitialiseGpuOpenGl	= rs_initialiseGpGpuWithOpenGlContexts;
    utils::logToD3					= rs_logToD3;
    utils::rsGetStreams				= rs_getStreams;
    utils::rsSendFrame				= rs_sendFrame;
    utils::rsGetFrameCamera			= rs_getFrameCamera;
    utils::rsAwaitFrameData			= rs_awaitFrameData;
    utils::rsShutdown				= rs_shutdown;

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
            Camera* cam = m_currentScene->getCurrentCamera();
            cam->setPosition(glm::vec3(res.camera.z, -res.camera.y, res.camera.x));
            utils::logToD3(std::to_string(res.camera.rx).c_str());
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

App* App::getInstance() 
{
	return s_instance;
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

    // hide window and set it to be current opengl context
	glfwHideWindow(m_window);
	glfwMakeContextCurrent(m_window);

    // initialise glew library, used to get openGL functions
	glewExperimental = GL_TRUE;
	glewInit();

    // enable gl depth testing and set to draw when the incoming depth value is less than the stored depth value
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	HGLRC wglContext = glfwGetWGLContext(m_window);
	HDC dc = GetDC(glfwGetWin32Window(m_window));

    m_currentScene = new Scene();

	LightSource* light = m_currentScene->addLightSource(glm::vec3(0, -5.f, 0), 1.f, .4f, VEC1);
	Object* sphere = m_currentScene->addObject(ObjectType::Sphere, ObjectArgs{ VEC0, 1.f, VEC1});
    m_currentScene->addObject(ObjectType::Sphere, ObjectArgs{ glm::vec3(-6.f, 0, 0), 1.f, glm::vec3(0, .7, 1) });

	if(utils::rsInitialiseGpuOpenGl(wglContext, dc))
        utils::error("failed to initialise RenderStream GPU interop");

	while(true)
	{
		if(handleStreams())
			break;

		if (sendFrames())
			break;

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
