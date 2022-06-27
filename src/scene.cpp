#include "scene.hpp"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "object.hpp"
#include "shape.hpp"
#include "utils.hpp"
#include "app.hpp"

Scene::Scene(App* app) : m_app(app), m_currentCamera(new Camera(this, glm::vec3(0, -12.f, 0)) )
{
	utils::ShaderProgramSource source = utils::parseShader("scene.shader");
	m_shader = utils::createShader(source.vertexSource, source.fragmentSource);
	glLinkProgram(m_shader);
	glUseProgram(m_shader);
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