#pragma once

#include <glm/matrix.hpp>
#include <vector>
#include <string>
#include <d3/d3renderstream.h>

#include "camera.hpp"
#include "utils.hpp"

#define VEC0 glm::vec3(0,0,0)
#define VEC1 glm::vec3(1,1,1)

class RsScene;
class Object;
class LightSource;

enum ObjectType {
    Object_Cube,
    Object_Sphere
};

struct ObjectArgs {
    std::string name;
    glm::vec3 pos = VEC0;
    float size = 1.0f;
    glm::vec3 colour = VEC1;
    int stackCount = 18;
    int sectorCount = 36;
};

class Scene {
    Camera* m_currentCamera;
    unsigned int m_shader;
    glm::mat4 m_view;
    glm::mat4 m_projection;
    std::vector<Object*> m_objects;
    std::vector<LightSource> m_lightSources;
    std::vector<Camera> m_cameras;
    RsScene* m_rsScene;
public:
    Scene(const char* name);
    ~Scene();
    void updateMatrices();
    void render();
    Object* addObject(ObjectType type, ObjectArgs args);
    unsigned int getShader();
    LightSource* addLightSource(glm::vec3 position, float brightness=0.0f, float ambientStrength=0.0f, glm::vec3 colour=VEC0);
    Camera* addCamera(glm::vec3 pos = VEC0, float fov = 45.f);
    Camera* getCurrentCamera();
    const char* getName();

    int getObjectCount();
    int getObjectCount(ObjectType type);
};