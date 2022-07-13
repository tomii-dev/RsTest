#include "camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include <sstream>

#include "scene.hpp"
#include "utils.hpp"

Camera::Camera(Scene* scene, glm::vec3 position, float fov)
	: m_scene		(scene),
	  m_position	(position),
	  m_front		(1, 0, 0),
	  m_up			(0, 1, 0),
	  m_fov			(fov)  
{}

glm::vec3 Camera::getPosition() 
{
	return m_position;
}

void Camera::setPosition(glm::vec3 pos) 
{
	m_position = pos;
	m_scene->updateMatrices();
}

void Camera::setRotation(float pitch, float yaw, float roll) {
	glm::vec3 rot(glm::vec3(pitch, yaw, roll) - m_rot);
	m_front = glm::rotate(m_front, glm::radians(1.f), rot);
	m_rot += rot;
	m_scene->updateMatrices();
}

float Camera::getFov() 
{
	return m_fov;
}

void Camera::setFov(float fov) 
{
	m_fov = fov;
}

glm::vec3 Camera::getFront() 
{
	return m_front;
}

glm::vec3 Camera::getUp() 
{
	return m_up;
}