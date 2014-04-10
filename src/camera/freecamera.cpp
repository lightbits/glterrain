#include <camera/freecamera.h>

void FreeCamera::reset(float theta, float phi, const vec3 &position)
{
	setHorizontalAngle(theta);
	setVerticalAngle(phi);
	setPosition(position);
}

void FreeCamera::update(Renderer &gfx, Context &ctx, double dt)
{
	if (ctx.isMousePressed(SDL_BUTTON_LEFT))
	{
		int wh = ctx.getWidth() / 2;
		int hh = ctx.getHeight() / 2;
		int dx = ctx.getMouseX() - wh;
		int dy = ctx.getMouseY() - hh;
		rotateHorizontal(dx * 0.4f * dt);
		rotateVertical(dy * 0.4f * dt);
		ctx.setMousePos(wh, hh);
		ctx.setCursorEnabled(false);
	}
	else
		ctx.setCursorEnabled(true);

	if (ctx.isKeyPressed(SDL_SCANCODE_LCTRL))
		moveDown(dt);
	else if (ctx.isKeyPressed(SDL_SCANCODE_SPACE))
		moveUp(dt);

	if (ctx.isKeyPressed('w'))
		moveForward(dt);
	else if (ctx.isKeyPressed('s'))
		moveBackward(dt);

	if (ctx.isKeyPressed('a'))
		moveLeft(dt);
	else if (ctx.isKeyPressed('d'))
		moveRight(dt);
}