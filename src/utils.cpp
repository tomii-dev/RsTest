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
#include "object.hpp"

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
    decltype(rs_getFrameImageData)* rsGetFrameImageData;
    decltype(rs_getFrameImage)* rsGetFrameImage;

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
        glCompileShader(vs);
        glAttachShader(program, vs);
        glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
        if (!compiled)
            logToD3("failed to compile vertex shader");

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, fsSrc, NULL);
        glCompileShader(fs);
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

    const char* glInternalFormatStr(GLint format)
    {
        switch (format)
        {
        case GL_RGBA8:
            return "GL_RGBA8";
        case GL_RGBA32F:
            return "GL_RGBA32F";
        case GL_RGBA16:
            return "GL_RGBA16";
        default:
            return "unknown format";
        }
    }

    const char* glFormatStr(GLint format)
    {
        switch(format) 
        {
        case GL_BGRA:
            return "GL_BGRA";
        case GL_RGBA:
            return "GL_RGBA";
        }
    }

    const char* glTypeStr(GLenum type)
    {
        switch (type)
        {
        case GL_UNSIGNED_BYTE:
            return "GL_UNSIGNED_BYTE";
        case GL_FLOAT:
            return "GL_FLOAT";
        case GL_UNSIGNED_SHORT:
            return "GL_UNSIGNED_SHORT";
        }
    }

    void lowerStr(std::string& str)
    {
        std::for_each(str.begin(), str.end(), [](char& c) {
            c = tolower(c);
        });
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

void RsSchema::removeScene(const RsScene& scene)
{
    m_scenes.erase(std::remove_if(m_scenes.begin(), m_scenes.end(), 
        [scene](const RemoteParameters& s) { return s.name == scene.name; }));
    scenes.scenes = &m_scenes[0];
    --scenes.nScenes;
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

void RsScene::addParam(RemoteParameter param)
{
    m_params.push_back(param);
    parameters = &m_params[0];
    ++nParameters;
}

void RsScene::removeParamsForObj(Object* obj)
{
    std::vector<RemoteParameter>::iterator it;
    for (it = m_params.begin(); it != m_params.end();)
        if (!strcmp(it->group, obj->getName()))
            it = m_params.erase(it);
        else ++it;

    parameters = m_params.size() ? &m_params[0] : nullptr;
    nParameters = m_params.size();
}

RsFloatParam::RsFloatParam(const std::string& key, const std::string& display,
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

RsTextureParam::RsTextureParam(const std::string& key, const std::string& display,
                                const std::string& group)
{
    this->group                     = _strdup(group.c_str());
    this->displayName               = _strdup(display.c_str());
    this->key                       = _strdup(key.c_str());
    this->type                      = RS_PARAMETER_IMAGE;

    nOptions                        = 0;
    options                         = nullptr;
    dmxOffset                       = -1;
    dmxType                         = RS_DMX_16_BE;
    flags                           = REMOTEPARAMETER_NO_FLAGS;
}

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ibo);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ibo);
}

void VertexArray::addVertex(v3 position, v2 texCoord, v3 normal)
{
    for (int i = 0; i < 3; ++i)
        m_vertices.push_back(position[i]);

    m_vertices.push_back(texCoord.x);
    m_vertices.push_back(texCoord.y);

    for (int i = 0; i < 3; ++i)
        m_vertices.push_back(normal[i]);
}

void VertexArray::addIndex(unsigned int ind) 
{ 
    m_indices.push_back(ind); 
}

void VertexArray::setIndices(const std::vector<unsigned int>& indices) 
{ 
    m_indices = indices;
}

void VertexArray::bind() { glBindVertexArray(m_vao); }

void VertexArray::build()
{
    bind();
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_vertices.size(), &m_vertices[0], GL_STATIC_DRAW);

    // set up position attrib
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);

    // set up tex coord attrib
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (const void*)12);

    // set up normal attrib
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (const void*)20);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * m_indices.size(), &m_indices[0], GL_STATIC_DRAW);
}

size_t VertexArray::getIndexCount()
{
    return m_indices.size();
}