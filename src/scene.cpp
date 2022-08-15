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
    in vec4 a_Position;
    in vec4 a_Normal;
    in vec2 a_TexCoord;

    out vec4 fragPos;
    out vec4 normal;
    out vec2 texCoord;
    out vec3 cubeMapTexCoord;

    uniform mat4 u_Model;
    uniform mat4 u_View;
    uniform mat4 u_Proj;

    void main() {
        fragPos = u_Model * a_Position;
        normal = u_Model * a_Normal;
        texCoord = a_TexCoord;
        cubeMapTexCoord = vec3(a_Position);
        gl_Position = u_Proj * u_View * fragPos;
    }
    )src" };

    const GLchar* fsSource[] = {R"src(#version 330 core
    in vec4 fragPos;
    in vec4 normal;
    in vec2 texCoord;
    in vec3 cubeMapTexCoord;

    uniform vec3 u_LightPos;
    uniform vec3 u_LightColour;
    uniform float u_LightBrightness;
    uniform int u_ObjectType;
    uniform vec3 u_ObjectColour;
    uniform float u_AmbientStrength;
    uniform bool u_IsTextured;
    uniform sampler2D u_Texture;
    uniform samplerCube u_CubeMap;

    void main(){
	    vec3 ambient = u_AmbientStrength * u_LightColour;
	    vec4 norm = normalize(normal);
	    vec4 texColour;
	    if (u_IsTextured)
            if(u_ObjectType == 0)
                texColour = texture(u_CubeMap, cubeMapTexCoord);
            else
		        texColour = texture(u_Texture, texCoord);
	    else
		    texColour = vec4(1, 1, 1, 1);
	    vec4 lightDir = normalize(vec4(u_LightPos, 1.f) - fragPos);
	    float diffuseStrength = max(dot(norm, lightDir), 0.0f) * u_LightBrightness;	
	    vec3 diffuse = diffuseStrength * u_LightColour;
	    vec3 result = (ambient + diffuse) * u_ObjectColour;
	    gl_FragColor = vec4(result, 1.0f) * texColour;
    }
    )src" };

    m_shader = utils::createShader(vsSource, fsSource);

	glUseProgram(m_shader);
    utils::checkGLError(" creating shader program");

    m_rsScene->name = name;

    // Parameters for scene light
    m_rsScene->addParam(RsFloatParam("lightpos_x", "posX", "light", 0, -100, 100, 0.1));
    m_rsScene->addParam(RsFloatParam("lightpos_y", "posY", "light", 5, -100, 100, 0.1));
    m_rsScene->addParam(RsFloatParam("lightpos_z", "posZ", "light", 0, -100, 100, 0.1));
    m_rsScene->addParam(RsFloatParam("lightcol_r", "colR", "light", 255, 0, 255, 1));
    m_rsScene->addParam(RsFloatParam("lightcol_g", "colG", "light", 255, 0, 255, 1));
    m_rsScene->addParam(RsFloatParam("lightcol_b", "colB", "light", 255, 0, 255, 1));
    m_rsScene->addParam(RsFloatParam("amb_strength", "ambient strength", "light", .4, 0, .5, 0.05));
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

    const unsigned int viewLoc = glGetUniformLocation(m_shader, "u_View");
    const unsigned int projLoc = glGetUniformLocation(m_shader, "u_Proj");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &m_view[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &m_projection[0][0]);
}

void Scene::render(){

    const std::vector<float>& params = App::getParams();

    if (!params.size())
        return;

    m_light.setPosition(glm::vec3(params[2], -params[1], params[0]));
    m_light.setColour(glm::vec3(params[3] / 255, params[4] / 255, params[5] / 255));
    m_light.setAmbientStrength(params[6]);
    m_light.setBrightness(params[7]);

    const unsigned int lightPosLoc = glGetUniformLocation(m_shader, "u_LightPos");
    const unsigned int lightColourLoc = glGetUniformLocation(m_shader, "u_LightColour");
    const unsigned int brightnessLoc = glGetUniformLocation(m_shader, "u_LightBrightness");
    const unsigned int ambientLoc = glGetUniformLocation(m_shader, "u_AmbientStrength");

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

        int ind = i * 6 + 8;

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

    m_rsScene->addParam(RsFloatParam(args.name + "pos_x", "posX", args.name, args.pos.x, -100, 100, 0.1));
    m_rsScene->addParam(RsFloatParam(args.name + "pos_y", "posY", args.name, args.pos.y, -100, 100, 0.1));
    m_rsScene->addParam(RsFloatParam(args.name + "pos_z", "posZ", args.name, args.pos.z, -100, 100, 0.1));
    m_rsScene->addParam(RsFloatParam(args.name + "rot_x", "rotX", args.name, 0, 0, 359, 1));
    m_rsScene->addParam(RsFloatParam(args.name + "rot_y", "rotY", args.name, 0, 0, 359, 1));
    m_rsScene->addParam(RsFloatParam(args.name + "rot_z", "rotZ", args.name, 0, 0, 359, 1));

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
