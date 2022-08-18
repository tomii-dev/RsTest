#include "lightsource.hpp"

#include "object.hpp"

LightSource::LightSource(glm::vec3 position, float brightness, float ambientStrength, glm::vec4 colour, Object* obj)
    : m_position        (position), 
    m_brightness        (brightness),
    m_colour            (colour),
    m_obj               (obj)
{}

void LightSource::render() 
{
    if (m_obj != nullptr) {
        m_obj->update();
        m_obj->draw();
    }
}

float LightSource::getBrightness()
{
    return m_brightness;
}

void LightSource::setBrightness(float brightness)
{
    m_brightness = brightness;
}

glm::vec3 LightSource::getPosition()
{
    return m_position;
}

void LightSource::setPosition(glm::vec3 position)
{
    m_position = position;
    if(m_obj != nullptr)
        m_obj->setPosition(position);
}

glm::vec4 LightSource::getColour()
{
    return m_colour;
}

void LightSource::setColour(glm::vec4 colour)
{
    m_colour = colour;
}