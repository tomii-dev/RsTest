#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x2.hpp>
#include <vector>
#include <map>
#include <string>

#include "object.hpp"

#define WHITE glm::vec3(1,1,1)

class Shape : public Object{
protected:
	unsigned int m_vao;
	unsigned int m_vbo;
	unsigned int m_ibo;
	unsigned int m_cbo;
	unsigned int m_nbo;
	unsigned int m_tbo;
	unsigned int m_shader;
	std::vector<float> m_vertices;
	std::vector<unsigned int> m_indices;
	std::vector<float> m_normals;
	std::vector<float> m_texCoords;
	float m_size;
	glm::vec3 m_colour;
	virtual void init();
public:
	Shape(Scene* scene, glm::vec3 pos, float size, glm::vec3 colour, const std::string& name);
	void update(const ImageFrameData& imgData) override;
	void draw() override;
};

class Cube : public Shape{
	void init() override;
public:
	Cube(Scene* scene, glm::vec3 pos, float size, const std::string& name, glm::vec3 colour=WHITE);
};

class Sphere : public Shape {
	int m_stackCount;
	int m_sectorCount;
	void init() override;
public:
	Sphere(Scene* scene, glm::vec3 pos, float size, const std::string& name, int stackCount, int sectorCount, glm::vec3 colour=WHITE);
	int getStacks();
	void setStacks(int count);
	int getSectors();
	void setSectors(int count);
};