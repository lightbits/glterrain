/*
A free-view type camera
*/

#ifndef FREE_CAMERA_H
#define FREE_CAMERA_H
#include <app/context.h>
#include <graphics/renderer.h>
#include <camera/camera.h>

class FreeCamera : public Camera
{
public:
	/* Reset view to horizontal angle 'theta', vertical angle 'phi', and
	spatial location 'position'*/
	void reset(float theta, float phi, const vec3 &position);

	/* Respond to player input */
	void update(Renderer &gfx, Context &ctx, float dt);
};

#endif