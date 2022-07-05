#include "scene.hpp"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "object.hpp"
#include "shape.hpp"
#include "utils.hpp"
#include "app.hpp"

Scene::Scene(App* app) : m_app(app), m_currentCamera(new Camera(this, glm::vec3(-10, 0, -1), 70.f) )
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

	glLinkProgram(m_shader);
	glUseProgram(m_shader);
    utils::checkGLError(" creating shader program");

    updateMatrices();
}

Scene::~Scene(){
    glDeleteShader(m_shader);
}

void Scene::updateMatrices() {
    float width = m_app->getWindowWidth();
    float height = m_app->getWindowHeight();
    glm::vec3 camPos(m_currentCamera->getPosition());
    glm::vec3 camFront(m_currentCamera->getFront());
    glm::vec3 camUp(m_currentCamera->getUp());
    glm::mat4 m_projection 
        = glm::perspective(glm::radians(m_currentCamera->getFov()), width / height, 0.1f, 9000.0f);
    glm::mat4 m_view = glm::lookAt(camPos, camPos + camFront, camUp);
    unsigned int viewLoc = glGetUniformLocation(m_shader, "u_View");
    unsigned int projLoc = glGetUniformLocation(m_shader, "u_Proj");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &m_view[0][0]);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &m_projection[0][0]);
    unsigned int mvpLoc = glGetUniformLocation(m_shader, "u_Mvp");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &(m_projection * m_view)[0][0]);
}

void Scene::update(){
    if (!m_lightSources.size()) {
        std::cout << "no light source!!\n";
        return;
    }
    unsigned int lightPosLoc = glGetUniformLocation(m_shader, "u_LightPos");
    unsigned int lightColourLoc = glGetUniformLocation(m_shader, "u_LightColour");
    unsigned int brightnessLoc = glGetUniformLocation(m_shader, "u_LightBrightness");
    unsigned int ambientLoc = glGetUniformLocation(m_shader, "u_AmbientStrength");
    glUniform3fv(lightPosLoc, 1, &m_lightSources[0].getPosition()[0]);
    glUniform3fv(lightColourLoc, 1, &m_lightSources[0].getColour()[0]);
    glUniform1f(ambientLoc, m_lightSources[0].getAmbientStrength());
    glUniform1f(brightnessLoc, m_lightSources[0].getBrightness());
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