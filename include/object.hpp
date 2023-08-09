#pragma once

#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <vector>

#include "lightsource.hpp"
#include "scene.hpp"
#include "utils.hpp"

#include "d3renderstream.h"

class Scene;

struct Texture
{
    GLuint id;
    GLenum target;
};

class Object
{
private:
    std::string m_name;
    glm::vec3 m_position;
    glm::vec3 m_size;
    Scene* m_scene;
    glm::mat4 m_model;
    glm::mat4 m_rotation;
    Texture m_texture;
    glm::vec2 m_lastTexSize;
protected:
    ObjectType m_type;
    VertexArray m_vao;
public:
    Object(const char* name);
    Object(Scene* scene, glm::vec3 pos, glm::vec3 size, const std::string& name);
    // take in image data to update texture
    virtual void update(const ImageFrameData& imgData = ImageFrameData());
    virtual void draw();
    void rotate(float deg, glm::vec3 dir);
    glm::vec3 getPosition();
    void setPosition(glm::vec3 pos);
    glm::vec3 getSize();
    ObjectType getType();
    void setSize(glm::vec3 size);
    void setRotation(float x, float y, float z);
    const char* getName();
};