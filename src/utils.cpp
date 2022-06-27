#include "utils.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace utils {

	stbi_uc* loadImgBuffer;
	unsigned int utilShader;

	ShaderProgramSource parseShader(const std::string& path) {
		std::ifstream stream(path);

		enum class ShaderType { NONE = -1, VERTEX = 0, FRAGMENT = 1 };

		std::string line;
		std::stringstream ss[2];
		ShaderType type = ShaderType::NONE;
		while (getline(stream, line)) {
			if (line.find("#shader") == std::string::npos) {
				ss[(int)type] << line << '\n';
				continue;
			}
			if (line.find("vertex") != std::string::npos)
				type = ShaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos)
				type = ShaderType::FRAGMENT;
		}

		return { ss[0].str(), ss[1].str() };
	}

	unsigned int compileShader(unsigned int type, const std::string& source)
	{
		unsigned int id = glCreateShader(type);
		const char* src = &source[0];
		glShaderSource(id, 1, &src, nullptr);
		glCompileShader(id);
		int res;
		glGetShaderiv(id, GL_COMPILE_STATUS, &res);
		if (!res)
		{
			int len;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
			char* msg = (char*)alloca(len * sizeof(char));
			glGetShaderInfoLog(id, len, &len, msg);
			std::cout << "failed to compile" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "shader" << std::endl;
			std::cout << msg << std::endl;
			glDeleteShader(id);
			return 0;
		}

		return id;
	}

	unsigned int createShader(const std::string& vertexShader, const std::string& fragmentShader)
	{
		unsigned int program = glCreateProgram();
		unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
		unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

		glAttachShader(program, vs);
		glAttachShader(program, fs);
		glLinkProgram(program);
		glValidateProgram(program);

		glDeleteShader(vs);
		glDeleteShader(fs);

		return program;
	}

	void setup() {
		ShaderProgramSource source = parseShader("debug.shader");
		utilShader = createShader(source.vertexSource, source.fragmentSource);
		glLinkProgram(utilShader);

		int w, h, c;
		stbi_set_flip_vertically_on_load(1);
		loadImgBuffer = stbi_load("load.jpg", &w, &h, &c, 4);
	}

	void drawDebugLine(glm::vec3 point1, glm::vec3 point2) {
		glUseProgram(utilShader);

		glm::mat4 projection = glm::perspective(glm::radians(35.0f), 640.f / 480.f, 0.1f, 100.0f);
		glm::mat4 view = glm::lookAt(
			glm::vec3(4, 3, 3),
			glm::vec3(0, 0, 0),
			glm::vec3(0, 1, 0)
		);
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 mvp = projection * view * model;

		unsigned int matId = glGetUniformLocation(utilShader, "u_Mvp");
		glUniformMatrix4fv(matId, 1, GL_FALSE, &mvp[0][0]);

		glBegin(GL_LINES);
		glVertex3f(point1.x, point1.y, point1.z);
		glVertex3f(point2.x, point2.y, point2.z);
		glEnd();
	}

	glm::vec3 normal(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3) {
		glm::vec3 cross = glm::cross(point1 - point2, point1 - point3);
		float m = sqrt(
			pow(cross.x, 2) + 
			pow(cross.y, 2) + 
			pow(cross.z, 2)
		);
		return glm::abs(glm::vec3(cross.x / m, cross.y / m, cross.z / m));
	}

	std::string vecStr(glm::vec3 vec) {
		std::stringstream ss;
		ss << vec.x << ", " << vec.y << ", " << vec.z << "\n";
		return ss.str();
	}

	void removeDup(std::vector<glm::vec3>& vec) {
		std::vector<glm::vec3> temp;
		for (glm::vec3 v : vec) {
			if (std::find(temp.begin(), temp.end(), v) != temp.end())
				continue;
			if (std::find(temp.begin(), temp.end(), -v) != temp.end())
				continue;
			temp.push_back(v);
		}
		vec = temp;
	}

	template<typename T>
	int getIndex(std::vector<T>& vec, T val) {
		typename std::vector<T>::iterator it;
		it = std::find(vec.begin(), vec.end(), val);
		if (it != vec.cend())
			return std::distance(vec.begin(), it);
		return -1;
	}

	template<typename T>
	void fillVec(std::vector<T>& vec, T val, int count) {
		for (int i = 0; i < count; ++i)
			vec.push_back(val);
	}

	void checkGLError()
	{
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR) {
			std::cout << gluErrorString(err) << '\n';
		}
	}

	float radToDeg(float rad){
		return rad * 180.0f / PI;
	}

	void drawLoadingScreen(GLFWwindow* window) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawPixels(1920, 1080, GL_RGBA, GL_UNSIGNED_BYTE, loadImgBuffer);
		glfwSwapBuffers(window);
	}
}