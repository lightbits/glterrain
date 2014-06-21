/*
A free-view type camera
*/

#ifndef FANCY_CAMERA_H
#define FANCY_CAMERA_H
#include <app/context.h>
#include <graphics/renderer.h>
#include <camera/camera.h>

class FancyCamera : public Camera
{
public:
	/* Reset view to horizontal angle 'theta', vertical angle 'phi', and
	spatial location 'position'*/
	void reset(float theta, float phi, const vec3 &position);

	/* Respond to player input */
	void update(Renderer &gfx, Context &ctx, float dt);

	mat4 getViewMatrix();
private:
	vec3 velocity;
	vec2 pointer;
	vec2 target;
	float speed;
	float roll_alpha;
	float roll;
	float max_roll;
	float roll_speed;
};

#endif