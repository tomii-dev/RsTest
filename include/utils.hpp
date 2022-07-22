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

#include "scene.hpp"

#define PI 3.1415926535897932384626433832795028841

#define STR( name ) # name

struct RenderTarget {
	GLuint texture;
	GLuint frameBuf;
};

enum ColourSpace
{
    COLOURSPACE_RGB,
    COLOURSPACE_SRGB,
};

typedef std::unordered_map<StreamHandle, RenderTarget> TargetMap;

static const char* colourSpaces[] = { "RGB", "sRGB" };
static const char* objectTypes[] = { "Cube", "Sphere" };

class RsScene : public RemoteParameters
{
    std::vector<RemoteParameter> m_params;
public:
    RsScene();
    void addParam(const RemoteParameter& param);
};

class RsSchema : public Schema
{
    std::vector<RemoteParameters> m_scenes;
public:
    RsSchema();
    // the scene reference param would be const but it got upset at me
    void addScene(RsScene& scene);
    void reloadScene(RsScene& scene);
};

class RsParam : public RemoteParameter
{
public:
    RsParam(const std::string& key, const std::string& display,
        const std::string& group, float defaultVal, float min = 0, float max = 255,
        float step = 1, const std::vector<std::string>& opt = {}, bool allowSequencing = true);
};

namespace utils {

	// rs functions
	extern decltype(rs_initialiseGpGpuWithOpenGlContexts)* rsInitialiseGpuOpenGl;
	extern decltype(rs_getStreams)* rsGetStreams;
	extern decltype(rs_sendFrame)* rsSendFrame;
	extern decltype(rs_getFrameCamera)* rsGetFrameCamera;
	extern decltype(rs_awaitFrameData)* rsAwaitFrameData;
	extern decltype(rs_logToD3)* logToD3;
	extern decltype(rs_shutdown)* rsShutdown;
    extern decltype(rs_setSchema)* rsSetSchema;
    extern decltype(rs_getFrameParameters)* rsGetFrameParams;

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