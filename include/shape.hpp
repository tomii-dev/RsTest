#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x2.hpp>
#include <vector>
#include <map>
#include <string>

#include "object.hpp"
#include "utils.hpp"

#define WHITE glm::vec3(1,1,1)

class Cube : public Object 
{
public:
    Cube(Scene* scene, glm::vec3 pos, float size, const std::string& name, glm::vec3 colour=WHITE);
};

class Sphere : public Object 
{
private:
    int m_stackCount;
    int m_sectorCount;
public:
    Sphere(Scene* scene, glm::vec3 pos, float size, const std::string& name, int stackCount, int sectorCount, glm::vec3 colour=WHITE);
    int getStacks();
    void setStacks(int count);
    int getSectors();
    void setSectors(int count);
};