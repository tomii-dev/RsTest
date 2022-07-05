#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#define VEC0 glm::vec3(4,3,4)

class Scene;

class Camera {
	Scene* m_scene;
	glm::vec3 m_position;
	glm::vec3 m_front;
	glm::vec3 m_up;
	glm::vec3 m_rot;
	float m_fov;
public:
	Camera(Scene* scene, glm::vec3 position=VEC0, float fov=45.f);
	glm::vec3 getPosition();
	glm::vec3 getFront();
	glm::vec3 getUp();
	void setPosition(glm::vec3 pos);
	void move(glm::vec3 dir);
	void setRotation(float pitch, float yaw, float roll);
	void moveForward(float amount=1.f);
	void moveBackward(float amount=1.f);
	void moveLeft(float amount=1.f);
	void moveRight(float amount=1.f);
	void moveUp(float amount=1.f);
	void moveDown(float amount=1.f);
	float getFov();
	void setFov(float fov);
};