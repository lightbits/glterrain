#include <camera/camera.h>
#include <common/helpers.h>

Camera::Camera() : theta(0), phi(0), position(0, 0, -1)
{
	updateVectors();
}

void Camera::rotateLeft(float t) { theta -= t; if(t < 0.0f) t += M_TWO_PI; }
void Camera::rotateRight(float t) { theta += t; if(t > M_TWO_PI) t -= M_TWO_PI; }
void Camera::rotateUp(float t) { if(phi < M_PI) phi += t; }
void Camera::rotateDown(float t) { if(phi > -M_PI) phi -= t; }

void Camera::setHorizontalAngle(float t) { theta = glm::mod(t, M_TWO_PI); }
void Camera::setVerticalAngle(float t) { phi = glm::mod(abs(t), M_PI) * (t < 0 ? -1 : 1); }
void Camera::setPosition(const glm::vec3 &p) { position = p; }

void Camera::updateVectors()
{
	float sintheta = sinf(theta);
	float costheta = cosf(theta);
	float sinphi = sinf(phi);
	float cosphi = cosf(phi);
	forward = vec3(cosphi * sintheta, sinphi, -cosphi * costheta);
	right = vec3(costheta, 0.0f, sintheta);
	up = glm::cross(right, forward);
}

mat4 Camera::getViewMatrix() { return glm::lookAt(position, position + forward, up); }
mat4 Camera::getViewMatrixFocus(float radius, const vec3 &focus) { return glm::lookAt(focus + radius * forward, focus, up); }