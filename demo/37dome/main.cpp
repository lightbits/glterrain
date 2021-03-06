#include "app.h"
#include <app/log.h>

int main(int argc, char **argv)
{
	GLContext ctx;
	if (!ctx.create(VideoMode(720, 480, 24, 0, 4), "Dome rendering", true, true))
	{
		APP_LOG << "Failed to open context\n";
		return EXIT_FAILURE;
	}
	APP_LOG << ctx.getDebugInfo();

	Renderer gfx;
	gfx.init(ctx);

	try
	{
		if (!load())
		{
			gfx.dispose();
			ctx.dispose();
			APP_LOG << "Failed to load content\n";
			return EXIT_FAILURE;
		}

		init(gfx, ctx);
		ctx.key_pressed = keyPressed;
		ctx.key_released = keyReleased;
		ctx.mouse_dragged = mouseDragged;
		ctx.mouse_moved = mouseMoved;
		ctx.mouse_pressed = mousePressed;
		ctx.mouse_released = mouseReleased;

		int updates_per_sec = 60;
		double tickrate = 1.0 / updates_per_sec;
		double accumulator = 0.0;
		double frametime = 0.0;
		while (ctx.isOpen())
		{
			double frame_begin = ctx.getElapsedTime();
			accumulator += frametime;
			while (accumulator >= tickrate)
			{
				update(gfx, ctx, tickrate);
				accumulator -= tickrate;
			}
			
			render(gfx, ctx, frametime);
			ctx.display();
			ctx.pollEvents();

			if (ctx.isKeyPressed(SDL_SCANCODE_ESCAPE))
				ctx.close();

			if (checkGLErrors())
				ctx.close();
			frametime = ctx.getElapsedTime() - frame_begin;
		}
	}
	catch (std::exception &e)
	{
		APP_LOG << "An unexpected error occurred: " << e.what();
	}

	free();
	gfx.dispose();
	ctx.dispose();
	return EXIT_SUCCESS;
}