#include "app.h"
#include <app/log.h>

int main(int argc, char **argv)
{
	GLContext ctx;
	if (!ctx.create(VideoMode(700, 700, 24, 0, 4, 4, 3), "33 3D fluid simulation", true, true))
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

		int updates_per_sec = 5;
		double tickrate = 1.0 / updates_per_sec;
		double accumulator = 0.0;
		double frametime = 0.0;
		while (ctx.isOpen())
		{
			double frame_t = ctx.getElapsedTime();
			double frame_begin = ctx.getElapsedTime();
			accumulator += frametime;
			int num_updates = 0;
			while (accumulator >= tickrate && num_updates < 3)
			{
				update(gfx, ctx, tickrate);
				accumulator -= tickrate;
				num_updates++;
			}
			
			render(gfx, ctx, frametime);
			ctx.display();
			ctx.pollEvents();

			if (ctx.isKeyPressed(SDL_SCANCODE_ESCAPE))
				ctx.close();

			if (checkGLErrors())
				ctx.close();
			frametime = ctx.getElapsedTime() - frame_t;
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