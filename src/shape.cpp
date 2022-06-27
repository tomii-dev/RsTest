#include "shape.hpp"
#include <GL/glew.h>
#include <iostream>
#include "utils.hpp"
#include "scene.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "texture.hpp"
#include "app.hpp"

Shape::Shape(Scene* scene, glm::vec3 pos, float size, glm::vec3 colour, std::string texPath)
	: Object		(scene, pos, glm::vec3(size)),
	  m_size		(size),
	  m_colour		(colour),
	  m_texture		(texPath)
{}

void Shape::init() {

	unsigned int shader = m_scene->getShader();

	// generate vertex array
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// generate vertex buffer
	unsigned int posLoc = glGetAttribLocation(shader, "a_Position");
	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*m_vertices.size(), m_vertices.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(posLoc);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

	// generate index buffer
	glGenBuffers(1, &m_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*m_indices.size(), m_indices.data(), GL_DYNAMIC_DRAW);

	// generate normal buffer
	unsigned int normLoc = glGetAttribLocation(shader, "a_Normal");
	glGenBuffers(1, &m_nbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_nbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*m_vertices.size(), m_normals.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(normLoc);
	glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

	// generate tex coord buffer
	unsigned int texCoordLoc = glGetAttribLocation(shader, "a_TexCoord");
	glGenBuffers(1, &m_tbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_tbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_texCoords.size(), m_texCoords.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(texCoordLoc);
	glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, 0);
}

void Shape::update() {

	Object::update();

	unsigned int shader = m_scene->getShader();

	glBindVertexArray(m_vao);

	// set colour uniform
	unsigned int colorLoc = glGetUniformLocation(shader, "u_ObjectColour");
	glUniform3fv(colorLoc, 1, &m_colour[0]);

	// bind and set texture
	unsigned int isTexLoc = glGetUniformLocation(shader, "u_IsTextured");
	if (m_texture.getWidth()) {
		m_texture.bind();
		unsigned int texLoc = glGetUniformLocation(shader, "u_Texture");
		glUniform1i(texLoc, 0);
		glUniform1i(isTexLoc, 1);
	}
	else glUniform1i(isTexLoc, 0);
}

void Shape::draw() {
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
	utils::checkGLError();
}

Cube::Cube(Scene* scene, glm::vec3 pos, float size, glm::vec3 colour, std::string texPath)
	: Shape(scene, pos, size, colour, texPath)
{
	init();
}

void Cube::init() {
	// temporary
	// TODO: write algorithm to calculate vertices from center point
	std::vector<float> vertices = {
		-1.0f,1.0f,1.0f,
		-1.0f,-1.0f,1.0f,
		1.0f,1.0f,1.0f,
		1.0f,-1.0f,1.0f,
		-1.0f,1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		1.0f,1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
	};

	m_vertices = vertices;

	std::vector<unsigned int> indices = {
		0, 2, 3,
		0, 3, 1,
		0, 1, 5,
		0, 4, 5,
		0, 2, 6,
		0, 4, 6,
		4, 5, 7,
		4, 6, 7,
		3, 2, 6,
		3, 7, 6,
		3, 1, 5,
		3, 7, 5
	};

	m_indices = indices;

	std::vector<float> normals = {
		0, 0, 1,
		0, 0, 1,
		1, 0, 0,
		1, 0, 0,
		-1, 0, 0,
		0, 0, -1,
		0, 1, 0,
		0, 0, -1
	};

	m_normals = normals;

	std::vector<float> texCoords = {
		0.0f, 0.0f,
		1.f, 0.f,
		1.f, 0.f,
		0.f, 0.f,
		1.f, 1.f,
		1.f, 1.f,
		0.f, 1.f,
		0.f, 1.f
	};

	m_texCoords = texCoords;

	Shape::init();
}

Sphere::Sphere(Scene* scene, glm::vec3 pos, float radius, int stackCount, int sectorCount, glm::vec3 colour, std::string texPath)
	: Shape				(scene, pos, radius * 2.f, colour, texPath),
	  m_stackCount		(stackCount),
	  m_sectorCount		(sectorCount)
{
	init();
}

void Sphere::init() {
	int stackIt = 0;
	int	secIt = 0;
	float stackStep = PI / m_stackCount;
	float sectorStep = 2 * PI / m_sectorCount;
	
	while (stackIt <= m_stackCount) {

		float stackAngle = PI / 2 - stackIt * stackStep;
		float sectorAngle = secIt * sectorStep;

		glm::vec3 pos(
			cosf(stackAngle) * cosf(sectorAngle),
			cosf(stackAngle) * sinf(sectorAngle),
			sinf(stackAngle)
		);

		for (int i = 0; i < 3; ++i) {
			m_vertices.push_back(pos[i]);
			m_normals.push_back(pos[i]);
		}

		// tex coords
		float s = (float)secIt / m_sectorCount;
		float t = (float)stackIt / m_stackCount;
		m_texCoords.push_back(s);
		m_texCoords.push_back(t);

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
				m_indices.push_back(stackBegin);
				m_indices.push_back(nextStack);
				m_indices.push_back(stackBegin + 1);
			}

			if (stackIt != m_stackCount - 1)
			{
				m_indices.push_back(stackBegin + 1);
				m_indices.push_back(nextStack);
				m_indices.push_back(nextStack + 1);
			}
		}
		stackIt++;
	}

	Shape::init();
}

int Sphere::getStacks() {
	return m_stackCount;
}

void Sphere::setStacks(int count) {
	m_stackCount = count;
	init();
}

int Sphere::getSectors() {
	return m_sectorCount;
}

void Sphere::setSectors(int count) {
	m_sectorCount = count;
	init();
}