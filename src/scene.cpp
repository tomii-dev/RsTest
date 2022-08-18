#include "scene.hpp"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>

#include "object.hpp"
#include "shape.hpp"
#include "utils.hpp"
#include "app.hpp"

Scene::Scene(const char* name) : m_currentCamera(new Camera(this, glm::vec3(-10, 0, -1))),
                                 m_rsScene      (new RsScene()),
                                 m_light        (glm::vec3(20.f, -15.f, 0.f), 1.f, .4f, VEC1)
{
    const GLchar* vsSource[] = {R"src(#version 330 core
    layout (location = 0) in vec4 aPosition;
    layout (location = 1) in vec2 aTexCoord;
    layout (location = 2) in vec4 aNormal;

    out vec4 fragPos;
    out vec4 normal;
    out vec2 texCoord;

    uniform mat4 uModel;
    uniform mat4 uView;
    uniform mat4 uProj;

    void main() {
        fragPos = uModel * aPosition;
        normal = uModel * aNormal;
        texCoord = vec2(1, 1) - aTexCoord;
        gl_Position = uProj * uView * fragPos;
    }
    )src" };

    const GLchar* fsSource[] = {R"src(#version 330 core
    in vec4 fragPos;
    in vec4 normal;
    in vec2 texCoord;

    uniform vec3 uLightPos;
    uniform vec3 uLightColour;
    uniform float uLightBrightness;
    uniform float uAmbientColour;
    uniform float uAmbientStrength;
    uniform bool uIsTextured;
    uniform sampler2D uTexture;

    void main(){
        vec3 ambient = uAmbientStrength * uAmbientColour;
        vec4 norm = normalize(normal);
        vec4 texColour;
        if (uIsTextured)
            texColour = texture(uTexture, texCoord);
        else
            texColour = vec4(1, 1, 1, 1);
        vec4 lightDir = normalize(vec4(uLightPos, 1.f) - fragPos);
        float diffuseStrength = max(dot(norm, lightDir), 0.0f) * uLightBrightness;	
        vec3 diffuse = diffuseStrength * uLightColour;
        vec3 result = ambient + diffuse;
        gl_FragColor = vec4(result, 1.0f) * texColour;
    }
    )src" };

    m_shader = utils::createShader(vsSource, fsSource);

    glUseProgram(m_shader);
    utils::checkGLError(" creating shader program");

    m_rsScene->name = name;

    // Parameters for scene light
    m_rsScene->addParam(RsFloatParam("lightpos_x", "pos_x", "light", 0, -100, 100, 0.1));
    m_rsScene->addParam(RsFloatParam("lightpos_y", "pos_y", "light", 5, -100, 100, 0.1));
    m_rsScene->addParam(RsFloatParam("lightpos_z", "pos_z", "light", 0, -100, 100, 0.1));
    m_rsScene->addParam(RsFloatParam("lightcol_r", "light_r", "light", 1, 0, 1, .01));
    m_rsScene->addParam(RsFloatParam("lightcol_g", "light_g", "light", 1, 0, 1, .01));
    m_rsScene->addParam(RsFloatParam("lightcol_b", "light_b", "light", 1, 0, 1, .01));
    m_rsScene->addParam(RsFloatParam("lightcol_a", "light_a", "light", 1, 0, 1, .01));
    m_rsScene->addParam(RsFloatParam("amb_strength", "ambient strength", "light", .4, 0, 1, 0.05));
    m_rsScene->addParam(RsFloatParam("brightness", "brightness", "light", 1, 0, 2, 0.1));

    App::getSchema().addScene(*m_rsScene);
    App::reloadSchema();

    updateMatrices();
}

Scene::~Scene(){
    glDeleteShader(m_shader);
}

void Scene::updateMatrices() {
    App* app = App::getInstance();
    const float width = app->getWindowWidth();
    const float height = app->getWindowHeight();
    const glm::vec3 camPos(m_currentCamera->getPosition());
    const glm::vec3 camFront(m_currentCamera->getFront());
    const glm::vec3 camUp(m_currentCamera->getUp());
    const glm::mat4 m_projection = glm::perspective(glm::radians(m_currentCamera->getFov()), width / height, 0.1f, 9000.0f);
    const glm::mat4 m_view = glm::lookAt(camPos, camPos + camFront, camUp);

    const unsigned int viewLoc = glGetUniformLocation(m_shader, "uView");
    const unsigned int projLoc = glGetUniformLocation(m_shader, "uProj");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &m_view[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &m_projection[0][0]);
}

void Scene::render(){

    const std::vector<float>& params = App::getParams();

    if (!params.size())
        return;

    m_light.setPosition(glm::vec3(params[2], -params[1], params[0]));
    m_light.setColour(glm::vec3(params[3], params[4], params[5]));
    m_light.setAmbientStrength(params[7]);
    m_light.setBrightness(params[8]);

    const unsigned int lightPosLoc = glGetUniformLocation(m_shader, "uLightPos");
    const unsigned int lightColourLoc = glGetUniformLocation(m_shader, "uLightColour");
    const unsigned int brightnessLoc = glGetUniformLocation(m_shader, "uLightBrightness");
    const unsigned int ambientLoc = glGetUniformLocation(m_shader, "uAmbientStrength");

    glUniform3fv(lightPosLoc, 1, &m_light.getPosition()[0]);
    glUniform3fv(lightColourLoc, 1, &m_light.getColour()[0]);
    glUniform1f(brightnessLoc, m_light.getBrightness());
    glUniform1f(ambientLoc, m_light.getAmbientStrength());

    if (!getObjectCount())
        return;

    const std::vector<ImageFrameData>& imgData = App::getImgData();

    for (int i = 0; i < m_objects.size(); ++i)
    {
        Object* obj = m_objects[i];

        int ind = i * 6 + 9;

        // set object position and rotation to values returned by frame parameters
        obj->setPosition(glm::vec3(params[ind + 2], -params[ind + 1], params[ind]));
        obj->setRotation(-params[ind + 5], params[ind + 3], -params[ind + 4]);

        obj->update(imgData[i]);

        obj->draw();
    }
}

Object* Scene::addObject(ObjectType type, ObjectArgs args){
    Object* obj;

    if (args.name == "")
    {
        args.name = objectTypes[type];
        args.name += " ";
        args.name += std::to_string(getObjectCount(type));
    }

    switch(type){
    case Object_Cube:
        obj = new Cube(this, args.pos, args.size, args.name.c_str(), args.colour);
        break;
    case Object_Sphere:
        obj = new Sphere(this, args.pos, args.size, args.name.c_str(), args.stackCount, args.sectorCount, args.colour);
        break;
    }

    m_objects.push_back(obj);

    m_rsScene->addParam(RsFloatParam(args.name + "pos_x", "pos_x", args.name, args.pos.x, -100, 100, 0.1));
    m_rsScene->addParam(RsFloatParam(args.name + "pos_y", "pos_y", args.name, args.pos.y, -100, 100, 0.1));
    m_rsScene->addParam(RsFloatParam(args.name + "pos_z", "pos_z", args.name, args.pos.z, -100, 100, 0.1));
    m_rsScene->addParam(RsFloatParam(args.name + "rot_x", "rot_x", args.name, 0, 0, 359, 1));
    m_rsScene->addParam(RsFloatParam(args.name + "rot_y", "rot_y", args.name, 0, 0, 359, 1));
    m_rsScene->addParam(RsFloatParam(args.name + "rot_z", "rot_z", args.name, 0, 0, 359, 1));

    m_rsScene->addParam(RsTextureParam(args.name + "texture", "texture", args.name));

    App::getSchema().reloadScene(*m_rsScene);
    App::reloadSchema();

    return obj;
}

void Scene::removeObject(Object* obj)
{
    m_objects.erase(std::remove(m_objects.begin(), m_objects.end(), obj));

    m_rsScene->removeParamsForObj(obj);
    delete obj;

    App::getSchema().reloadScene(*m_rsScene);
    App::reloadSchema();
}

unsigned int Scene::getShader(){
    return m_shader;
}

Camera* Scene::getCurrentCamera() {
    return m_currentCamera;
}

const std::vector<Object*>& Scene::getObjects()
{
    return m_objects;
}

const std::vector<Camera*>& Scene::getCameras()
{
    return m_cameras;
}

int Scene::getObjectCount()
{
    return m_objects.size();
}

int Scene::getObjectCount(ObjectType type)
{
    int count = 0;
    for (Object* o : m_objects)
        if (o->getType() == type)
            count++;
    return count;
}

Object* Scene::operator [](int i)
{
    return m_objects[i];
}
