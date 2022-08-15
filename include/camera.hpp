#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <string>

#define VEC0 glm::vec3(4,3,4)

class Scene;

class Camera {
private:
    std::string m_name;
    Scene* m_scene;
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_rot;
    float m_fov;
public:
    Camera(Scene* scene, glm::vec3 position=VEC0, float fov=45.f);
    glm::vec3 getPosition();
    glm::vec3 getFront();
    glm::vec3 getUp();
    void setPosition(glm::vec3 pos);
    void setRotation(float pitch, float yaw, float roll);
    float getFov();
    void setFov(float fov);
};