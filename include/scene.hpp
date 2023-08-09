#pragma once

#include <glm/matrix.hpp>
#include <vector>
#include <string>
#include <d3renderstream.h>

#include "camera.hpp"
#include "utils.hpp"
#include "lightsource.hpp"

#if !defined(VEC0)
#define VEC0 glm::vec3(0,0,0)
#endif
#if !defined(VEC1)
#define VEC1 glm::vec3(1,1,1)
#endif

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
private:
    std::string m_name;
    Camera* m_currentCamera;
    unsigned int m_shader;
    glm::mat4 m_view;
    glm::mat4 m_projection;
    std::vector<Object*> m_objects;
    LightSource m_light;
    std::vector<Camera*> m_cameras;
    RsScene* m_rsScene;
    float m_ambStrength;
    glm::vec4 m_ambColour;
public:
    Scene(std::string name);
    ~Scene();
    void updateMatrices();
    void render();
    Object* addObject(ObjectType type, ObjectArgs args);
    void removeObject(Object* obj);
    unsigned int getShader();
    Camera* addCamera(glm::vec3 pos = VEC0, float fov = 45.f);
    Camera* getCurrentCamera();
    const char* getName();

    const std::vector<Object*>& getObjects();
    const std::vector<Camera*>& getCameras();

    int getObjectCount();
    int getObjectCount(ObjectType type);

    Object* operator [](int i);
};