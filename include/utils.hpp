#pragma once

#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/GLU.h>
#endif
#ifdef _WIN32
#include <GL/GLU.h>
#endif
#include <glm/vec3.hpp>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <d3/d3renderstream.h>
#include <string>
#include <vector>
#include <unordered_map>

#define PI 3.1415926535897932384626433832795028841

#define STR( name ) # name

struct RenderTarget {
	GLuint texture;
	GLuint frameBuf;
};

typedef std::unordered_map<StreamHandle, RenderTarget> TargetMap;

namespace utils {

	// rs functions
	extern decltype(rs_initialiseGpGpuWithOpenGlContexts)* rsInitialiseGpuOpenGl;
	extern decltype(rs_getStreams)* rsGetStreams;
	extern decltype(rs_sendFrame)* rsSendFrame;
	extern decltype(rs_getFrameCamera)* rsGetFrameCamera;
	extern decltype(rs_awaitFrameData)* rsAwaitFrameData;
	extern decltype(rs_logToD3)* logToD3;
	extern decltype(rs_shutdown)* rsShutdown;

	struct ShaderProgramSource {
		std::string vertexSource;
		std::string fragmentSource;
	};

    int error(const std::string& msg="");

	// create shader and return program id
	unsigned int createShader(const GLchar* vsSrc[], const GLchar* fsSrc[]);

	void checkGLError(const std::string& add = "");

	const StreamDescriptions* getStreams(std::vector<uint8_t>& desc);
	GLint glInternalFormat(RSPixelFormat format);
	GLint glFormat(RSPixelFormat format);
	GLenum glType(RSPixelFormat format);

    const std::string& rsErrorStr(RS_ERROR);
}