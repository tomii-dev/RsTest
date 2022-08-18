#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class Object;

class LightSource{
private:
    float m_brightness;
    glm::vec3 m_position;
    glm::vec4 m_colour;
    Object* m_obj;
public:
    LightSource(glm::vec3 position, float brightness=0.0f, float ambientStrength=0.0f, glm::vec4 colour=glm::vec4(), Object* obj=nullptr);
    void render();
    float getBrightness();
    void setBrightness(float brightness);
    glm::vec3 getPosition();
    void setPosition(glm::vec3 position);
    glm::vec4 getColour();
    void setColour(glm::vec4 colour);
};