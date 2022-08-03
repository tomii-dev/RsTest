#include "object.hpp"
#include "scene.hpp"
#include <iostream>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

Object::Object(){}

Object::Object(Scene* scene, glm::vec3 pos, glm::vec3 size)
    : m_scene       (scene),
      m_position    (pos),
      m_size        (size),
      m_rotation    (1.0f),
      m_texture     (0)
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
    const unsigned int modelLoc = glGetUniformLocation(m_scene->getShader(), "u_Model");
    m_model = glm::translate(glm::mat4(1.0f), m_position)
        * m_rotation
        * glm::scale(glm::mat4(1.0f), m_size);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &m_model[0][0]);

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

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexImage2D(GL_TEXTURE_2D, 0, utils::glInternalFormat(imgData.format), size.x, size.y,
        0, utils::glFormat(imgData.format), utils::glType(imgData.format), nullptr);
    utils::checkGLError();
    SenderFrameTypeData data;
    data.gl.texture = m_texture;
    if (utils::rsGetFrameImage(imgData.imageId, RS_FRAMETYPE_OPENGL_TEXTURE, data))
        utils::logToD3("failed to get texture param info");
    const GLint texLoc = glGetUniformLocation(m_scene->getShader(), "u_Texture");
    glUniform1i(texLoc, 0);

    m_lastTexSize = size;
}

void Object::draw()
{}

void Object::rotate(float deg, glm::vec3 dir) 
{
    m_rotation = glm::rotate(m_rotation, glm::radians(deg), dir);
}

void Object::init() {}