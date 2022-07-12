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

#include "app.hpp"

namespace utils {

	decltype(rs_initialiseGpGpuWithOpenGlContexts)* rsInitialiseGpuOpenGl;
	decltype(rs_getStreams)* rsGetStreams;
	decltype(rs_sendFrame)* rsSendFrame;
	decltype(rs_getFrameCamera)* rsGetFrameCamera;
	decltype(rs_awaitFrameData)* rsAwaitFrameData;
	decltype(rs_logToD3)* logToD3;
	decltype(rs_shutdown)* rsShutdown;

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

	const StreamDescriptions* getStreams(std::vector<uint8_t>& desc) {		
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

	GLint glInternalFormat(RSPixelFormat format) {
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

	GLint glFormat(RSPixelFormat format) {
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

	GLenum glType(RSPixelFormat format) {
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
}