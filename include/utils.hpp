#pragma once

#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/GLU.h>
#endif
#ifdef _WIN32
#include <GL/GLU.h>
#endif
#include <glm/vec3.hpp>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <string>
#include <vector>

#define PI 3.1415926535897932384626433832795028841

namespace utils {

	enum VboType {
		VBO_2D = 2,
		VBO_3D = 3
	};

	struct ShaderProgramSource {
		std::string vertexSource;
		std::string fragmentSource;
	};

	extern stbi_uc* loadImgBuffer;

	// return ShaderProgramSource for shader passed by path
	ShaderProgramSource parseShader(const std::string& path);

	unsigned int compileShader(unsigned int type, const std::string& source);

	// create shader and return program id
	unsigned int createShader(const std::string& vertexShader, const std::string& fragmentShader);

	void setup();

	void drawDebugLine(glm::vec3 point1, glm::vec3 point2);
	glm::vec3 normal(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3);
	std::string vecStr(glm::vec3 vec);

	// remove duplicate vectors (x, y, z) from vector (data structure)
	void removeDup(std::vector<glm::vec3>& vec);

	// get index of value in vector
	template<typename T>
	int getIndex(std::vector<T>& vec, T val);

	template<typename T>
	void fillVec(std::vector<T>& vec, T val, int count);

	void checkGLError();
	float radToDeg(float rad);

	void drawLoadingScreen(GLFWwindow* window);
}