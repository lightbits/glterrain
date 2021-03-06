#include "app.h"
#include <app/log.h>

int main(int argc, char **argv)
{
	GLContext ctx;
	if (!ctx.create(VideoMode(720, 480, 24, 0, 4), "Arealights", true, true))
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

		int updates_per_sec = 80;
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
			
			render(gfx, ctx, tickrate);
			ctx.display();
			ctx.pollEvents();

			if (ctx.isKeyPressed(SDL_SCANCODE_ESCAPE))
				ctx.close();

			if (checkGLErrors() > 0)
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