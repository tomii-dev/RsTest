#include "shape.hpp"
#include <GL/glew.h>
#include <iostream>
#include "utils.hpp"
#include "scene.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "app.hpp"

Cube::Cube(Scene* scene, glm::vec3 pos, float size, const std::string& name, glm::vec3 colour)
    : Object(scene, pos, glm::vec3(size), name)
{
    m_type = Object_Cube;
    
    m_vao.addVertex(v3(-1,  1,  1 ), v2(1, 0), v3(0, 0, 1)); 
    m_vao.addVertex(v3(-1,  1,  1 ), v2(0, 0), v3(0, 0, 1)); 
    m_vao.addVertex(v3(-1,  1,  1 ), v2(0, 0), v3(0, 0, 1)); 

    m_vao.addVertex(v3(-1, -1,  1 ), v2(1, 1), v3(0, 0, 1)); 
    m_vao.addVertex(v3(-1, -1,  1 ), v2(0, 1), v3(0, 0, 1)); 
    m_vao.addVertex(v3(-1, -1,  1 ), v2(0, 1), v3(0, 0, 1)); 

    m_vao.addVertex(v3( 1,  1,  1 ), v2(0, 0), v3(1, 0, 0)); 
    m_vao.addVertex(v3( 1,  1,  1 ), v2(1, 0), v3(1, 0, 0)); 
    m_vao.addVertex(v3( 1,  1,  1 ), v2(1, 0), v3(1, 0, 0)); 

    m_vao.addVertex(v3( 1,  -1,  1), v2(0, 1), v3(1, 0, 0)); 
    m_vao.addVertex(v3( 1,  -1,  1), v2(1, 1), v3(1, 0, 0)); 
    m_vao.addVertex(v3( 1,  -1,  1), v2(1, 1), v3(1, 0, 0)); 

    m_vao.addVertex(v3(-1,  1, -1), v2(1, 0), v3(-1, 0, 0)); 
    m_vao.addVertex(v3(-1,  1, -1), v2(0, 1), v3(-1, 0, 0)); 
    m_vao.addVertex(v3(-1,  1, -1), v2(0, 0), v3(-1, 0, 0)); 

    m_vao.addVertex(v3(-1, -1, -1), v2(1, 1), v3(0, 0, -1)); 
    m_vao.addVertex(v3(-1, -1, -1), v2(0, 1), v3(0, 0, -1)); 
    m_vao.addVertex(v3(-1, -1, -1), v2(0, 0), v3(0, 0, -1)); 

    m_vao.addVertex(v3( 1,  1, -1 ), v2(1, 1), v3(0, 1, 0));
    m_vao.addVertex(v3( 1,  1, -1 ), v2(1, 0), v3(0, 1, 0)); 
    m_vao.addVertex(v3( 1,  1, -1 ), v2(0, 0), v3(0, 1, 0)); 

    m_vao.addVertex(v3( 1, -1, -1), v2(1, 1), v3(0, 0, -1)); 
    m_vao.addVertex(v3( 1, -1, -1), v2(0, 1), v3(0, 0, -1)); 
    m_vao.addVertex(v3( 1, -1, -1), v2(1, 0), v3(0, 0, -1)); 

    m_vao.setIndices({
        0, 6, 9, 0, 9, 3,
        1, 4, 15, 1, 12, 15,
        2, 7, 18, 2, 13, 18,
        14, 16, 21, 14, 19, 21,
        10, 8, 20, 10, 22, 20,
        11, 5, 17, 11, 23, 17
    });

    m_vao.build();
}

Sphere::Sphere(Scene* scene, glm::vec3 pos, float radius, const std::string& name, int stackCount, int sectorCount, glm::vec3 colour)
    : Object		    (scene, pos, glm::vec3(radius * 2.f), name),
      m_stackCount		(stackCount),
      m_sectorCount		(sectorCount)
{
    m_type = Object_Sphere;

    int stackIt = 0;
    int	secIt = 0;
    float stackStep = PI / m_stackCount;
    float sectorStep = 2 * PI / m_sectorCount;

    while (stackIt <= m_stackCount) {
        float stackAngle = PI / 2 - stackIt * stackStep;
        float sectorAngle = secIt * sectorStep;

        v3 pos(
            cosf(stackAngle) * cosf(sectorAngle),
            cosf(stackAngle) * sinf(sectorAngle),
            sinf(stackAngle)
        );

        v3 normal(pos);

        // tex coords
        v2 texCoord(
            (float)secIt / m_sectorCount,
            (float)stackIt / m_stackCount
        );

        m_vao.addVertex(pos, texCoord, normal);

        if (secIt == m_sectorCount) {
            secIt = 0;
            stackIt++;
            continue;
        }

        secIt++;
    }

    stackIt = 0;

    while (stackIt < m_stackCount) {
        int stackBegin = stackIt * (m_sectorCount + 1);
        int nextStack = stackBegin + m_sectorCount + 1;

        for (int j = 0; j < m_sectorCount; ++j, ++stackBegin, ++nextStack)
        {
            if (stackIt)
            {
                m_vao.addIndex(stackBegin);
                m_vao.addIndex(nextStack);
                m_vao.addIndex(stackBegin + 1);
            }

            if (stackIt != m_stackCount - 1)
            {
                m_vao.addIndex(stackBegin + 1);
                m_vao.addIndex(nextStack);
                m_vao.addIndex(nextStack + 1);
            }
        }
        stackIt++;
    }

    m_vao.build();
}

int Sphere::getStacks() {
    return m_stackCount;
}

void Sphere::setStacks(int count) {
    m_stackCount = count;
}

int Sphere::getSectors() {
    return m_sectorCount;
}

void Sphere::setSectors(int count) {
}