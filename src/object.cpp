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
      m_rotation    (1.0f)
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

void Object::update()
{
    const unsigned int modelLoc = glGetUniformLocation(m_scene->getShader(), "u_Model");
    m_model = glm::translate(glm::mat4(1.0f), m_position)
        * m_rotation
        * glm::scale(glm::mat4(1.0f), m_size);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &m_model[0][0]);
}

void Object::draw(){}

void Object::rotate(float deg, glm::vec3 dir) 
{
    m_rotation = glm::rotate(m_rotation, glm::radians(deg), dir);
}