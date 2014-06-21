#include <camera/fancycamera.h>
#include <common/noise.h>

void FancyCamera::reset(float theta, float phi, const vec3 &position)
{
	setHorizontalAngle(theta);
	setVerticalAngle(phi);
	setPosition(position);
	roll_alpha = 0.0f;
	roll = 0.0f;
	roll_speed = 2.5f;
	max_roll = 0.3f;
	velocity = vec3(0.0f);
	speed = 10.0f;
	pointer = vec2(0.0f);
	target = vec2(0.0f);
}

void FancyCamera::update(Renderer &gfx, Context &ctx, float dt)
{
	int wh = ctx.getWidth() / 2;
	int hh = ctx.getHeight() / 2;
	if (ctx.isMousePressed(SDL_BUTTON_LEFT))
	{
		target = vec2(ctx.getMousePos());
		pointer += 0.85f * (target - pointer) * dt;
		vec2 delta = pointer - vec2(wh, hh);
		rotateHorizontal(delta.x * 0.4f * dt);
		rotateVertical(delta.y * 0.4f * dt);
		ctx.setMousePos(wh, hh);
		ctx.setCursorEnabled(false);
	}
	else
	{
		pointer = vec2(wh, hh);
		ctx.setCursorEnabled(true);
	}

	if (ctx.isKeyPressed(SDL_SCANCODE_LCTRL))
	{
		if (velocity.y > -speed)
			velocity.y -= speed * 0.5f * dt;
	}
	else if (ctx.isKeyPressed(SDL_SCANCODE_SPACE))
	{
		if (velocity.y < speed)
			velocity.y += speed * 0.5f * dt;
	}
	else
	{
		velocity.y *= 0.88f;
	}

	if (ctx.isKeyPressed('w'))
	{
		if (velocity.z < speed)
			velocity.z += speed * 0.5f * dt;
	}
	else if (ctx.isKeyPressed('s'))
	{
		if (velocity.z > -speed)
			velocity.z -= speed * 0.5f * dt;
	}
	else
	{
		velocity.z *= 0.88f;
	}

	if (ctx.isKeyPressed('a'))
	{
		if (roll_alpha > -1.0f)
			roll_alpha -= roll_speed * dt;
		if (velocity.x > -speed)
			velocity.x -= speed * 0.5f * dt;
	}
	else if (ctx.isKeyPressed('d'))
	{
		if (roll_alpha < 1.0f)
			roll_alpha += roll_speed * dt;
		if (velocity.x < speed)
			velocity.x += speed * 0.5f * dt;
	}
	else
	{
		velocity.x *= 0.88f;
		roll_alpha *= 0.88f;
	}

	moveForward(velocity.z * dt);
	moveRight(velocity.x * dt);
	moveUp(velocity.y * dt);
	rotateVertical(-velocity.z * 0.001f);
}

mat4 FancyCamera::getViewMatrix()
{
	float sign = roll_alpha > 0.0f ? 1.0f : -1.0f;
	return Camera::getViewMatrix() * transform::rotateZ(sign * sin(abs(roll_alpha * PI / 2.0f)) * max_roll);
}