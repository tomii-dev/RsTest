#include "object.hpp"
#include "scene.hpp"
#include <iostream>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "app.hpp"

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

glm::vec3 Object::getSize()
{
    return m_size;
}

void Object::setSize(glm::vec3 size) 
{
    m_size = size;
}

void Object::setRotation(float x, float y, float z)
{
    m_rotation = glm::eulerAngleXYZ(glm::radians(x), glm::radians(y), glm::radians(z));
}

void Object::update(const ImageFrameData& imgData)
{
    m_vao.bind();

    const GLint modelLoc = glGetUniformLocation(m_scene->getShader(), "uModel");
    m_model = glm::translate(glm::mat4(1.0f), m_position)
        * m_rotation
        * glm::scale(glm::mat4(1.0f), m_size);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &m_model[0][0]);

    // bind and set texture
    const GLint isTexLoc = glGetUniformLocation(m_scene->getShader(), "uIsTextured");

    const glm::vec2 size(imgData.width, imgData.height);

    if (!size.x)
    {
        glUniform1i(isTexLoc, 0);
        return;
    }

    glUniform1i(isTexLoc, 1);
    glBindTexture(m_texture.target, m_texture.id);

    if (size == m_lastTexSize)
        return;

    const GLint internalFormat  = utils::glInternalFormat(imgData.format);
    const GLenum format         = utils::glFormat(imgData.format);
    const GLenum type           = utils::glType(imgData.format);

    glGenTextures(1, &m_texture.id);
    glBindTexture(GL_TEXTURE_2D, m_texture.id);
    m_texture.target = GL_TEXTURE_2D;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, imgData.width, imgData.height, 0, format, type, nullptr);

    SenderFrameTypeData data;
    data.gl.texture = m_texture.id;
    if (utils::rsGetFrameImage(imgData.imageId, RS_FRAMETYPE_OPENGL_TEXTURE, data))
        utils::logToD3(MSG(failed to get texture param info));
    const GLint texLoc = glGetUniformLocation(m_scene->getShader(), "uTexture");
    glUniform1i(texLoc, 0);

    m_lastTexSize = size;
}

void Object::draw()
{
    glDrawElements(GL_TRIANGLES, m_vao.getIndexCount(), GL_UNSIGNED_INT, nullptr);
}

void Object::rotate(float deg, glm::vec3 dir) 
{
    m_rotation = glm::rotate(m_rotation, glm::radians(deg), dir);
}

ObjectType Object::getType()
{
    return m_type;
}

const char* Object::getName()
{
    return m_name.c_str();
}