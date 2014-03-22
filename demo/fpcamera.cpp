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
	if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_1))
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

	if (glfwGetKey(GLFW_KEY_LCTRL))
		camera.moveDown(dt);
	else if (glfwGetKey(GLFW_KEY_SPACE))
		camera.moveUp(dt);

	if (glfwGetKey('W'))
		camera.moveForward(dt);
	else if (glfwGetKey('S'))
		camera.moveBackward(dt);

	if (glfwGetKey('A'))
		camera.moveLeft(dt);
	else if (glfwGetKey('D'))
		camera.moveRight(dt);
}

mat4 getCameraView()
{
	return camera.getViewMatrix();
}