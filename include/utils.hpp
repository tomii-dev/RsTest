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

typedef glm::vec2 v2;
typedef glm::vec3 v3;

class Object;

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
private:
    std::vector<RemoteParameter> m_params;
public:
    RsScene();
    void addParam(RemoteParameter param);
    void removeParamsForObj(Object* obj);
};

class RsSchema : public Schema
{
private:
    std::vector<RemoteParameters> m_scenes;
public:
    RsSchema();
    // the scene reference param would be const but it got upset at me
    void addScene(RsScene& scene);
    void removeScene(const RsScene& scene);
    void reloadScene(RsScene& scene);
};

class RsFloatParam : public RemoteParameter
{
public:
    RsFloatParam(const std::string& key, const std::string& display,
        const std::string& group, float defaultVal, float min = 0, float max = 255,
        float step = 1, const std::vector<std::string>& opt = {}, bool allowSequencing = true);
};

class RsTextureParam : public RemoteParameter
{
public:
    RsTextureParam(const std::string& key, const std::string& display,
        const std::string& group);
};

class VertexArray
{
private:
    unsigned int m_vao;
    unsigned int m_vbo;
    unsigned int m_ibo;
    std::vector<float> m_vertices;
    std::vector<unsigned int> m_indices;
public:
    VertexArray();
    ~VertexArray();
    void addVertex(v3 position, v2 texCoord, v3 normal);
    void addIndex(unsigned int ind);
    void setIndices(const std::vector<unsigned int>& indices);
    void bind();

    // take all information and generate buffers for GL
    void build();

    size_t getIndexCount();
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
    extern decltype(rs_getFrameImageData)* rsGetFrameImageData;
    extern decltype(rs_getFrameImage)* rsGetFrameImage;

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

    const char* glInternalFormatStr(GLint format);
    const char* glFormatStr(GLint format);
    const char* glTypeStr(GLenum type);
}