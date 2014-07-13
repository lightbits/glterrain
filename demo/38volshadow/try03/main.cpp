#include "app.h"
#include <app/log.h>

int main(int argc, char **argv)
{
	GLContext ctx;
	if (!ctx.create(VideoMode(720, 480, 24, 0, 0, 4, 3, false, false), "Compute shader", true, true))
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

		double dt = 0.0;
		int updates_per_sec = 60;
		double tickrate = 1.0 / updates_per_sec;
		double accumulator = 0.0;
		while (ctx.isOpen())
		{
			double frame_t = ctx.getElapsedTime();
			accumulator += dt;
			while (accumulator >= tickrate)
			{
				update(gfx, ctx, tickrate);
				printf("\r%.2f\t", dt * 1000.0);
				accumulator -= tickrate;
			}			
			
			render(gfx, ctx, dt);
			ctx.display();
			ctx.pollEvents();

			if (ctx.isKeyPressed(SDL_SCANCODE_ESCAPE))
				ctx.close();

			if (checkGLErrors())
				ctx.close();
			dt = ctx.getElapsedTime() - frame_t;
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