#pragma once

#include <glm/vec3.hpp>

class Object;

class LightSource{
    float m_brightness;
    float m_ambientStrength;
    glm::vec3 m_position;
    glm::vec3 m_colour;
    Object* m_obj;
public:
    LightSource(glm::vec3 position, float brightness=0.0f, float ambientStrength=0.0f, glm::vec3 colour=glm::vec3(), Object* obj=nullptr);
    void render();
    float getBrightness();
    float getAmbientStrength();
    void setAmbientStrength(float strength);
    void setBrightness(float brightness);
    glm::vec3 getPosition();
    void setPosition(glm::vec3 position);
    glm::vec3 getColour();
    void setColour(glm::vec3 colour);
};