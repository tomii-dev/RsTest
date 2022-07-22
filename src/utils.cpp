#include "utils.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <windows.h>
#include <locale>
#include <codecvt>
#include <sstream>

#include "scene.hpp"
#include "app.hpp"

namespace utils {

	decltype(rs_initialiseGpGpuWithOpenGlContexts)* rsInitialiseGpuOpenGl;
	decltype(rs_getStreams)* rsGetStreams;
	decltype(rs_sendFrame)* rsSendFrame;
	decltype(rs_getFrameCamera)* rsGetFrameCamera;
	decltype(rs_awaitFrameData)* rsAwaitFrameData;
	decltype(rs_logToD3)* logToD3;
	decltype(rs_shutdown)* rsShutdown;
    decltype(rs_setSchema)* rsSetSchema;
    decltype(rs_getFrameParameters)* rsGetFrameParams;

    const std::string rsErrorStrs[] = {
           "RS_ERROR_SUCCESS",
           "RS_NOT_INITIALISED",
           "RS_ERROR_ALREADYINITIALISED",
           "RS_ERROR_INVALIDHANDLE",
           "RS_MAXSENDERSREACHED",
           "RS_ERROR_BADSTREAMTYPE",
           "RS_ERROR_NOTFOUND",
           "RS_ERROR_INCORRECTSCHEMA",
           "RS_ERROR_INVALID_PARAMETERS",
           "RS_ERROR_BUFFER_OVERFLOW",
           "RS_ERROR_TIMEOUT",
           "RS_ERROR_STREAMS_CHANGED",
           "RS_ERROR_INCOMPATIBLE_VERSION",
           "RS_ERROR_FAILED_TO_GET_DXDEVICE_FROM_RESOURCE",
           "RS_ERROR_FAILED_TO_INITIALISE_GPGPU",
           "RS_ERROR_QUIT",
           "RS_ERROR_UNSPECIFIED"
    };

    int error(const std::string& msg)
    {
        if (logToD3) logToD3(msg.c_str());
        const std::wstring wStr = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(msg);
        MessageBox(NULL, wStr.c_str(), L"RsTest Error :(", MB_OK);
        return 1;
    }

	unsigned int createShader(const GLchar* vsSrc[], const GLchar* fsSrc[])
	{
		unsigned int program = glCreateProgram();
		GLint compiled = GL_FALSE;

		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, vsSrc, NULL);
		glAttachShader(program, vs);
		glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
			logToD3("failed to compile vertex shader");

		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs, 1, fsSrc, NULL);
		glAttachShader(program, fs);
		glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
			logToD3("failed to compile frag shader");

		glLinkProgram(program);
		glValidateProgram(program);

		glDeleteShader(vs);
		glDeleteShader(fs);

		return program;
	}
	 
	void checkGLError(const std::string& add)
    {
		GLenum err;
		while (err = glGetError()) {
            std::stringstream ss;
            ss << gluErrorString(err) << " " << add;
			logToD3(ss.str().c_str());
		}
	}

	const StreamDescriptions* getStreams(std::vector<uint8_t>& desc) 
    {		
		uint32_t bytes;
		rsGetStreams(nullptr, &bytes);

		RS_ERROR res = RS_ERROR_BUFFER_OVERFLOW;
		const static int MAX_TRIES = 3;
		int it = 0;

		while (res == RS_ERROR_BUFFER_OVERFLOW && it < MAX_TRIES) {
			desc.resize(bytes);
			res = rsGetStreams(reinterpret_cast<StreamDescriptions*>(&desc[0]), &bytes);
			++it;
		}

		if (res != RS_ERROR_SUCCESS)
			throw std::runtime_error("issue getting streams :(");
		if (bytes < sizeof(StreamDescriptions))
			throw std::runtime_error("invalid stream descriptions :(");

		return reinterpret_cast<const StreamDescriptions*>(&desc[0]);
	}

	GLint glInternalFormat(RSPixelFormat format) 
    {
		switch (format) {
		case RS_FMT_BGRA8:
		case RS_FMT_BGRX8:
			return GL_RGBA8;
		case RS_FMT_RGBA32F:
			return GL_RGBA32F;
		case RS_FMT_RGBA16:
			return GL_RGBA16;
		case RS_FMT_RGBA8:
		case RS_FMT_RGBX8:
			return GL_RGBA8;
		default:
			throw std::runtime_error("Unhandled RS pixel format");
		}
	}

	GLint glFormat(RSPixelFormat format) 
    {
		switch (format) {
		case RS_FMT_BGRA8:
		case RS_FMT_BGRX8:
			return GL_BGRA;
		case RS_FMT_RGBA32F:
		case RS_FMT_RGBA16:
		case RS_FMT_RGBA8:
		case RS_FMT_RGBX8:
			return GL_RGBA;
		default:
			throw std::runtime_error("Unhandled RS pixel format");
		}
	}

	GLenum glType(RSPixelFormat format) 
    {
		switch (format)
		{
		case RS_FMT_BGRA8:
		case RS_FMT_BGRX8:
			return GL_UNSIGNED_BYTE;
		case RS_FMT_RGBA32F:
			return GL_FLOAT;
		case RS_FMT_RGBA16:
			return GL_UNSIGNED_SHORT;
		case RS_FMT_RGBA8:
		case RS_FMT_RGBX8:
			return GL_UNSIGNED_BYTE;
		default:
			throw std::runtime_error("Unhandled RS pixel format");
		}
	}

    const std::string& rsErrorStr(RS_ERROR err) 
    {
        return rsErrorStrs[err];
    }
}

RsSchema::RsSchema() 
{
    channels.nChannels  = 0;
    scenes.nScenes      = 0;
    channels.channels   = nullptr;
    scenes.scenes       = nullptr;
}

void RsSchema::addScene(RsScene& scene)
{
    m_scenes.push_back(scene);
    scenes.scenes = &m_scenes[0];
    ++scenes.nScenes;
}

void RsSchema::reloadScene(RsScene& scene)
{
    for (RemoteParameters& s : m_scenes)
        if (s.name == scene.name)
            s = scene;
    scenes.scenes = &m_scenes[0];
}

RsScene::RsScene()
{
    parameters = nullptr;
    nParameters = 0;
    
}

void RsScene::addParam(const RemoteParameter& param)
{
    m_params.push_back(param);
    parameters = &m_params[0];
    ++nParameters;
}

RsParam::RsParam(const std::string& key, const std::string& display,
    const std::string& group, float defaultVal, float min, float max, float step,
    const std::vector<std::string>& opt, bool allowSequencing)
{
    if (!opt.empty())
    {
        min = 0;
        max = float(opt.size() - 1);
        step = 1;
    }

    this->group                     = _strdup(group.c_str());
    this->displayName               = _strdup(display.c_str());
    this->key                       = _strdup(key.c_str());
    this->type                      = RS_PARAMETER_NUMBER;
    defaults.number.defaultValue    = defaultVal;
    defaults.number.min             = min;
    defaults.number.max             = max;
    defaults.number.step            = step;
    nOptions                        = uint32_t(opt.size());
    options                         = static_cast<const char**>(malloc(nOptions * sizeof(const char*)));

    for (size_t j = 0; j < opt.size(); ++j)
    {
        options[j] = _strdup(opt[j].c_str());
    }

    dmxOffset                       = -1; // Auto
    dmxType                         = RS_DMX_16_BE;
    flags                           = REMOTEPARAMETER_NO_FLAGS;

    if (!allowSequencing)
        flags |= REMOTEPARAMETER_NO_SEQUENCE;
}