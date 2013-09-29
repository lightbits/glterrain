#ifndef CAMERA_H
#define CAMERA_H
#include <common/matrix.h>
#include <common/vec.h>

class Camera
{
public:
	Camera();

	void rotateLeft(float t);
	void rotateRight(float t);
	void rotateUp(float t);
	void rotateDown(float t);

	void setHorizontalAngle(float t);
	void setVerticalAngle(float t);
	void setPosition(const vec3 &p);

	void updateVectors();

	mat4 getViewMatrix();
	mat4 getViewMatrixFocus(float radius, const vec3 &focus);
private:
	float theta;
	float phi;
	vec3 position;
	vec3 up;
	vec3 right;
	vec3 forward;
};

#endif