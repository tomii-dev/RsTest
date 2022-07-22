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
                                 m_rsScene      (new RsScene())
{
    const GLchar* vsSource[] = {R"src(#version 120
    attribute vec4 a_Position;
    attribute vec4 a_Normal;
    attribute vec2 a_TexCoord;

    varying vec4 fragPos;
    varying vec4 normal;
    varying vec2 texCoord;

    uniform mat4 u_Model;
    uniform mat4 u_View;
    uniform mat4 u_Proj;

    void main() {
        fragPos = u_Model * a_Position;
        normal = u_Model * a_Normal;
        texCoord = a_TexCoord;
        gl_Position = u_Proj * u_View * fragPos;
    }
    )src" };

    const GLchar* fsSource[] = {R"src(#version 120
    varying vec4 fragPos;
    varying vec4 normal;
    varying vec2 texCoord;

    uniform vec3 u_LightPos;
    uniform vec3 u_LightColour;
    uniform float u_LightBrightness;
    uniform vec3 u_ObjectColour;
    uniform float u_AmbientStrength;
    uniform bool u_IsTextured;
    uniform sampler2D u_Texture;

    void main(){
	    vec3 ambient = u_AmbientStrength * u_LightColour;
	    vec4 norm = normalize(normal);
	    vec4 texColour;
	    if (u_IsTextured)
		    texColour = texture2D(u_Texture, texCoord);
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
    const unsigned int mvpLoc = glGetUniformLocation(m_shader, "u_Mvp");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &(m_projection * m_view)[0][0]);
}

void Scene::update(){
    if (!m_lightSources.size()) {
        std::cout << "no light source!!\n";
        return;
    }
    const unsigned int lightPosLoc = glGetUniformLocation(m_shader, "u_LightPos");
    const unsigned int lightColourLoc = glGetUniformLocation(m_shader, "u_LightColour");
    const unsigned int brightnessLoc = glGetUniformLocation(m_shader, "u_LightBrightness");
    const unsigned int ambientLoc = glGetUniformLocation(m_shader, "u_AmbientStrength");

    glUniform3fv(lightPosLoc, 1, &m_lightSources[0].getPosition()[0]);
    glUniform3fv(lightColourLoc, 1, &m_lightSources[0].getColour()[0]);
    glUniform1f(brightnessLoc, m_lightSources[0].getBrightness());
    glUniform1f(ambientLoc, m_lightSources[0].getAmbientStrength());

    const std::vector<float>& params = App::getParams();
    std::vector<Object*>::iterator it;
    for (it = m_objects.begin(); it != m_objects.end(); ++it)
    {
        const int ind = !(it - m_objects.begin()) ? 0 : it - m_objects.begin() + 5;
        (*it)->setPosition(glm::vec3(params[ind + 2], -params[ind + 1], params[ind]));
        (*it)->setRotation(-params[ind + 5], params[ind + 3], -params[ind + 4]);
    }
}

void Scene::render(){
    for (Object* obj : m_objects) {
        obj->update();
        obj->draw();
    }
    for (LightSource& l : m_lightSources)
        l.render();
}

Object* Scene::addObject(ObjectType type, ObjectArgs args){
    Object* obj;
    switch(type){
    case ObjectType::Cube:
        obj = new Cube(this, args.pos, args.size, args.colour, args.texPath);
        break;
    case ObjectType::Sphere:
        obj = new Sphere(this, args.pos, args.size, args.stackCount, args.sectorCount, args.colour, args.texPath);
        break;
    }
    m_objects.push_back(obj);

    if (args.name == "")
    {
        args.name = objectTypes[(int)type];
        args.name += " ";
        args.name += std::to_string(getObjectCount());
    }

    // add remote parameters for object
    std::vector<RsParam> params;
    params.push_back(RsParam(args.name + "pos_x", "posX", args.name, args.pos.x, -1000, 1000, 0.1));
    params.push_back(RsParam(args.name + "pos_y", "posY", args.name, args.pos.y, -1000, 1000, 0.1));
    params.push_back(RsParam(args.name + "pos_z", "posZ", args.name, args.pos.z, -1000, 1000, 0.1));
    params.push_back(RsParam(args.name + "rot_x", "rotX", args.name, 0, 0, 359, 1));
    params.push_back(RsParam(args.name + "rot_y", "rotY", args.name, 0, 0, 359, 1));
    params.push_back(RsParam(args.name + "rot_z", "rotZ", args.name, 0, 0, 359, 1));

    for (const RsParam& param : params)
        m_rsScene->addParam(param);

    App::getSchema().reloadScene(*m_rsScene);
    App::reloadSchema();

    return obj;
}

unsigned int Scene::getShader(){
    return m_shader;
}

LightSource* Scene::addLightSource(glm::vec3 position, float brightness, float ambientStrength, glm::vec3 colour){
    Cube* lightCube = new Cube(this, position, .02f);
    LightSource light(position, brightness, ambientStrength, colour, lightCube);
    m_lightSources.push_back(light);
    return &m_lightSources[m_lightSources.size() - 1];
}

Camera* Scene::addCamera(glm::vec3 pos, float fov) {
    Camera balls(this);
    return &balls;
}

Camera* Scene::getCurrentCamera() {
    return m_currentCamera;
}

int Scene::getObjectCount()
{
    return m_objects.size();
}
