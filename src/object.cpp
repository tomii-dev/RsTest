#include "object.hpp"
#include "scene.hpp"
#include <iostream>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

Object::Object(const char* name) : m_name (name) {}

Object::Object(Scene* scene, glm::vec3 pos, glm::vec3 size, const std::string& name)
    : m_scene       (scene),
      m_position    (pos),
      m_size        (size),
      m_rotation    (1.0f),
      m_name        (name)
{}

glm::vec3 Object::getPosition()
{
    return m_position;
}

void Object::setPosition(glm::vec3 pos) 
{
    m_position = pos;
}

float Object::getSize() 
{
    return m_size.x;
}

void Object::setSize(float size) 
{
    m_size = glm::vec3(size);
}

void Object::setRotation(float x, float y, float z)
{
    m_rotation = glm::eulerAngleXYZ(glm::radians(x), glm::radians(y), glm::radians(z));
}

void Object::update(const ImageFrameData& imgData)
{
    const GLint modelLoc = glGetUniformLocation(m_scene->getShader(), "u_Model");
    m_model = glm::translate(glm::mat4(1.0f), m_position)
        * m_rotation
        * glm::scale(glm::mat4(1.0f), m_size);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &m_model[0][0]);

    const GLint typeLoc = glGetUniformLocation(m_scene->getShader(), "u_ObjectType");
    glUniform1i(typeLoc, (int)m_type);

    // bind and set texture
    const GLint isTexLoc = glGetUniformLocation(m_scene->getShader(), "u_IsTextured");

    const glm::vec2 size(imgData.width, imgData.height);

    if (!size.x)
    {
        glUniform1i(isTexLoc, 0);
        return;
    }

    glUniform1i(isTexLoc, 1);
    if (size == m_lastTexSize)
        return;

    const GLint internalFormat  = utils::glInternalFormat(imgData.format);
    const GLenum format         = utils::glFormat(imgData.format);
    const GLenum type           = utils::glType(imgData.format);

    Texture tmp;
    glGenTextures(1, &tmp.id);
    glBindTexture(GL_TEXTURE_2D, tmp.id);
    tmp.target = GL_TEXTURE_2D;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.x, size.y, 0, format, type, nullptr);

    SenderFrameTypeData data;
    data.gl.texture = tmp.id;
    if (utils::rsGetFrameImage(imgData.imageId, RS_FRAMETYPE_OPENGL_TEXTURE, data))
        utils::logToD3("failed to get texture param info");
    const GLint texLoc = glGetUniformLocation(m_scene->getShader(), "u_Texture");
    glUniform1i(texLoc, 0);

    // Note: Could use polymorphism and perform texture setup specific to object in its own update
    // overload, however after trying this I decided it would make things more complicated than they
    // need to be and a simple switch in this method works fine
    switch (m_type)
    {
    case Object_Cube:
    {
        // Texture needs to be 1:1 for cube map
        const GLsizei texSize = size.x > size.y ? size.x : size.y;

        // Get pixel data from texture provided by RS
        // TODO: flip pixel data
        GLubyte* pxData = new GLubyte[texSize * texSize * 4];
        glGetTexImage(GL_TEXTURE_2D, 0, format, type, pxData);

        // Unbind texture
        glBindTexture(GL_TEXTURE_2D, 0);

        // we want to use a cube map for texturing cube
        glGenTextures(1, &m_texture.id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture.id);
        m_texture.target = GL_TEXTURE_CUBE_MAP;

        for (uint32_t i = 0; i < 6; ++i)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, internalFormat, texSize, texSize, 0, format, type, pxData);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        break;
    }
    default:
        m_texture = tmp;
        break;
    }

    m_lastTexSize = size;
}

void Object::draw()
{}

void Object::rotate(float deg, glm::vec3 dir) 
{
    m_rotation = glm::rotate(m_rotation, glm::radians(deg), dir);
}

void Object::init() {}

ObjectType Object::getType()
{
    return m_type;
}

const char* Object::getName()
{
    return m_name.c_str();
}