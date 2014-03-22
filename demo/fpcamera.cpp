#include "fpcamera.h"
#include <camera/camera.h>

mat4 view_matrix;
Camera camera;

void resetCamera(float theta, float phi, vec3 position)
{
	camera.setHorizontalAngle(theta);
	camera.setVerticalAngle(phi);
	camera.setPosition(position);
}

void updateCamera(Renderer &gfx, Context &ctx, double dt)
{
	if (ctx.isMousePressed(SDL_BUTTON_LEFT))
	{
		int wh = ctx.getWidth() / 2;
		int hh = ctx.getHeight() / 2;
		int dx = ctx.getMouseX() - wh;
		int dy = ctx.getMouseY() - hh;
		camera.rotateHorizontal(dx * 0.4f * dt);
		camera.rotateVertical(dy * 0.4f * dt);
		ctx.setMousePos(wh, hh);
		ctx.setCursorEnabled(false);
	}
	else
		ctx.setCursorEnabled(true);

	if (ctx.isKeyPressed(SDL_SCANCODE_LCTRL))
		camera.moveDown(dt);
	else if (ctx.isKeyPressed(SDL_SCANCODE_SPACE))
		camera.moveUp(dt);

	if (ctx.isKeyPressed('w'))
		camera.moveForward(dt);
	else if (ctx.isKeyPressed('s'))
		camera.moveBackward(dt);

	if (ctx.isKeyPressed('a'))
		camera.moveLeft(dt);
	else if (ctx.isKeyPressed('d'))
		camera.moveRight(dt);
}

mat4 getCameraView()
{
	return camera.getViewMatrix();
}