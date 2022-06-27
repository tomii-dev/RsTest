#include "camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "scene.hpp"

Camera::Camera(Scene* scene, glm::vec3 position, float fov)
	: m_scene		(scene),
	  m_position	(position),
	  m_front		(1, 0, 0),
	  m_up			(0, 1, 0),
	  m_yawPitch	(0, 0),
	  m_fov			(fov)
{}

glm::vec3 Camera::getPosition() {
	return m_position;
}

void Camera::setPosition(glm::vec3 pos) {
	m_position = pos;
	m_scene->updateMatrices();
}

void Camera::move(glm::vec3 dir) {
	m_position += dir;
	m_scene->updateMatrices();
}

void Camera::rotate(glm::vec2 dir) {
	m_yawPitch += dir;
	m_front = glm::normalize(glm::vec3(
		cos(glm::radians(m_yawPitch.x)) * cos(glm::radians(m_yawPitch.y)),
		sin(glm::radians(m_yawPitch.y)),
		sin(glm::radians(m_yawPitch.x)) * cos(glm::radians(m_yawPitch.y))
	));
	m_scene->updateMatrices();
}

void Camera::moveForward(float amount) { move(m_front * amount); }
void Camera::moveBackward(float amount) { move(-m_front * amount); }

void Camera::moveLeft(float amount) { 
	move(-glm::normalize(glm::cross(m_front, m_up)) * amount); 
}

void Camera::moveRight(float amount) {
	move(glm::normalize(glm::cross(m_front, m_up)) * amount);
}
void Camera::moveUp(float amount){ move(m_up * amount); }
void Camera::moveDown(float amount){ move(-m_up * amount); }

float Camera::getFov() {
	return m_fov;
}

void Camera::setFov(float fov) {
	m_fov = fov;
}

glm::vec3 Camera::getFront() {
	return m_front;
}

glm::vec3 Camera::getUp() {
	return m_up;
}