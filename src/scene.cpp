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

Scene::Scene(std::string name) : m_currentCamera(new Camera(this, glm::vec3(-10, 0, -1))),
                                 m_rsScene      (new RsScene()),
                                 m_light        (glm::vec3(20.f, -15.f, 0.f), 1.f, .4f, v4(1.f)),
                                 m_name         (name)
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
    uniform vec4 uLightColour;
    uniform float uLightBrightness;
    uniform vec4 uAmbientColour;
    uniform float uAmbientStrength;
    uniform bool uIsTextured;
    uniform sampler2D uTexture;

    void main(){
        vec4 ambient = uAmbientStrength * uAmbientColour;
        vec4 norm = normalize(normal);
        vec4 texColour;
        if (uIsTextured)
            texColour = texture(uTexture, texCoord);
        else
            texColour = vec4(1, 1, 1, 1);
        vec4 lightDir = normalize(vec4(uLightPos, 1.f) - fragPos);
        float diffuseStrength = max(dot(norm, lightDir), 0.0f) * uLightBrightness;	
        vec4 diffuse = diffuseStrength * uLightColour;
        vec4 result = ambient + diffuse;
        gl_FragColor = result * texColour;
    }
    )src" };

    m_shader = utils::createShader(vsSource, fsSource);

    glUseProgram(m_shader);
    utils::checkGLError(" creating shader program");

    m_rsScene->name = m_name.c_str();

    const std::string nameStr(name);

    // ambient light params
    m_rsScene->addParam(RsFloatParam(nameStr + "amb_strength", "ambient strength", "scene", .4, 0, 1, 0.05)); // 0
    m_rsScene->addParam(RsFloatParam(nameStr + "ambcol_r", "ambient colour_r", "scene", 1, 0, 1, .01)); // 1
    m_rsScene->addParam(RsFloatParam(nameStr + "ambcol_g", "ambient colour_g", "scene", 1, 0, 1, .01)); // 2
    m_rsScene->addParam(RsFloatParam(nameStr + "ambcol_b", "ambient colour_b", "scene", 1, 0, 1, .01)); // 3
    m_rsScene->addParam(RsFloatParam(nameStr + "ambcol_a", "ambientcolour_a", "scene", 1, 0, 1, .01)); // 4

    // parameters for scene light
    m_rsScene->addParam(RsFloatParam(nameStr + "lightpos_x", "pos_x", "light", 0, -100, 100, 0.1)); // 5
    m_rsScene->addParam(RsFloatParam(nameStr + "lightpos_y", "pos_y", "light", 5, -100, 100, 0.1)); // 6
    m_rsScene->addParam(RsFloatParam(nameStr + "lightpos_z", "pos_z", "light", 0, -100, 100, 0.1)); // 7
    m_rsScene->addParam(RsFloatParam(nameStr + "lightcol_r", "light colour_r", "light", 1, 0, 1, .01)); // 8
    m_rsScene->addParam(RsFloatParam(nameStr + "lightcol_g", "light colour_g", "light", 1, 0, 1, .01)); // 9
    m_rsScene->addParam(RsFloatParam(nameStr + "lightcol_b", "light colour_b", "light", 1, 0, 1, .01)); // 10
    m_rsScene->addParam(RsFloatParam(nameStr + "lightcol_a", "light colour_a", "light", 1, 0, 1, .01)); // 11
    m_rsScene->addParam(RsFloatParam(nameStr + "brightness", "brightness", "light", 1, 0, 2, 0.1)); // 12

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

    m_ambStrength = params[0];
    m_ambColour = v4(params[1], params[2], params[3], params[4]);

    m_light.setPosition(v3(params[7], -params[6], params[5]));
    m_light.setColour(v4(params[8], params[9], params[10], params[11]));
    m_light.setBrightness(params[12]);

    const unsigned int ambientStrenLoc = glGetUniformLocation(m_shader, "uAmbientStrength");
    const unsigned int ambientColLoc = glGetUniformLocation(m_shader, "uAmbientColour");
    const unsigned int lightPosLoc = glGetUniformLocation(m_shader, "uLightPos");
    const unsigned int lightColourLoc = glGetUniformLocation(m_shader, "uLightColour");
    const unsigned int brightnessLoc = glGetUniformLocation(m_shader, "uLightBrightness");

    glUniform1f(ambientStrenLoc, m_ambStrength);
    glUniform4fv(ambientColLoc, 1, &m_ambColour[0]);
    glUniform3fv(lightPosLoc, 1, &m_light.getPosition()[0]);
    glUniform4fv(lightColourLoc, 1, &m_light.getColour()[0]);
    glUniform1f(brightnessLoc, m_light.getBrightness());

    if (!getObjectCount())
        return;

    const std::vector<ImageFrameData>& imgData = App::getImgData();

    for (int i = 0; i < m_objects.size(); ++i)
    {
        Object* obj = m_objects[i];

        int ind = i * 9 + 13;

        // set object position, rotation, scale to values returned by frame parameters
        obj->setPosition(glm::vec3(params[ind + 2], -params[ind + 1], params[ind]));
        obj->setRotation(-params[ind + 5], params[ind + 3], -params[ind + 4]);
        obj->setSize(v3(params[ind + 6], params[ind + 7], params[ind + 8]));
        obj->update(imgData[i]);

        obj->draw();
    }
}

Object* Scene::addObject(ObjectType type, ObjectArgs args){
    Object* obj;

    if (args.name == "")
    {
        std::string objName = objectTypes[type];
        utils::lowerStr(objName);

        args.name = objName + " ";
        args.name += std::to_string(getObjectCount(type) + 1);
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

    // use prefix to identify object by its scene and name
    const std::string prefix = m_name + args.name;

    m_rsScene->addParam(RsFloatParam(prefix + "pos_x", "pos_x", args.name, args.pos.x, -100, 100, 0.1));
    m_rsScene->addParam(RsFloatParam(prefix + "pos_y", "pos_y", args.name, args.pos.y, -100, 100, 0.1));
    m_rsScene->addParam(RsFloatParam(prefix + "pos_z", "pos_z", args.name, args.pos.z, -100, 100, 0.1));
    m_rsScene->addParam(RsFloatParam(prefix + "rot_x", "rot_x", args.name, 0, 0, 359, 1));
    m_rsScene->addParam(RsFloatParam(prefix + "rot_y", "rot_y", args.name, 0, 0, 359, 1));
    m_rsScene->addParam(RsFloatParam(prefix + "rot_z", "rot_z", args.name, 0, 0, 359, 1));
    m_rsScene->addParam(RsFloatParam(prefix + "scale_x", "scale_x", args.name, 1, 0, 10, .01));
    m_rsScene->addParam(RsFloatParam(prefix + "scale_y", "scale_y", args.name, 1, 0, 10, .01));
    m_rsScene->addParam(RsFloatParam(prefix + "scale_z", "scale_z", args.name, 1, 0, 10, .01));

    m_rsScene->addParam(RsTextureParam(prefix + "texture", "texture", args.name));

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
