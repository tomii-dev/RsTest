#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x2.hpp>
#include <vector>
#include <map>
#include <string>

#include "object.hpp"
#include "texture.hpp"

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
	Texture m_texture;
	std::vector<float> m_vertices;
	std::vector<unsigned int> m_indices;
	std::vector<float> m_normals;
	std::vector<float> m_texCoords;
	float m_size;
	glm::vec3 m_colour;
	virtual void init();
public:
	Shape(Scene* scene, glm::vec3 pos, float size, glm::vec3 colour, std::string texPath);
	void update() override;
	void draw() override;
};

class Cube : public Shape{
	void init() override;
public:
	Cube(Scene* scene, glm::vec3 pos, float size, glm::vec3 colour=WHITE, std::string texPath="");
};

class Sphere : public Shape {
	int m_stackCount;
	int m_sectorCount;
	void init() override;
public:
	Sphere(Scene* scene, glm::vec3 pos, float size, int stackCount, int sectorCount, glm::vec3 colour=WHITE, std::string texPath = "");
	int getStacks();
	void setStacks(int count);
	int getSectors();
	void setSectors(int count);
};