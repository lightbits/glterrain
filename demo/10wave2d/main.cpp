#include "app.h"
#include <app/log.h>
#include <video/videocapture.h>

int main(int argc, char **argv)
{
	Log log("log.txt", true);
	GLContext ctx;
	if (!ctx.create(VideoMode(1280, 720, 24, 0, 4), "Wave Equation 2D", true, true))
	{
		log << "Failed to open context\n";
		return EXIT_FAILURE;
	}
	log << ctx.getDebugInfo();

	Renderer gfx;
	gfx.init(ctx);

	try
	{
		if (!load())
		{
			gfx.dispose();
			ctx.dispose();
			log << "Failed to load content\n";
			return EXIT_FAILURE;
		}

		init(gfx, ctx);
		int i = 0;

		int updates_per_sec = 200;
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

			if (ctx.isKeyPressed('t'))
			{
				std::stringstream ss;
				ss << "screenshot" << i << ".bmp";
				ctx.screenshot(ss.str().c_str());
				i++;
			}

			if (checkGLErrors(log) > 0)
				ctx.close();
			frametime = ctx.getElapsedTime() - frame_begin;
		}
	}
	catch (std::exception &e)
	{
		log << "An unexpected error occurred: " << e.what();
	}

	free();
	gfx.dispose();
	ctx.dispose();
	return EXIT_SUCCESS;
}