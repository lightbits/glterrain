#include "app.h"
#include <app/log.h>

int main(int argc, char **argv)
{
	GLContext ctx;
	if (!ctx.create(VideoMode(720, 480, 24, 0, 4), "Quaternions", true, true))
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

		int updates_per_sec = 60;
		double target_frame_time = 1.0 / 60.0;
		double secs_per_update = 1.0 / double(updates_per_sec);
		double accumulator = 0.0;
		double dt = 0.0;
		double prev_t = 0.0;
		while (ctx.isOpen())
		{
			double t = ctx.getElapsedTime();
			double dt = t - prev_t;
			prev_t = t;

			t = ctx.getElapsedTime();
			accumulator += dt;
			while (accumulator >= secs_per_update)
			{
				update(gfx, ctx, secs_per_update);
				accumulator -= secs_per_update;
			}
			double update_time = ctx.getElapsedTime() - t;
			
			t = ctx.getElapsedTime();
			render(gfx, ctx, dt);
			ctx.display();
			ctx.pollEvents();

			if (ctx.isKeyPressed(SDL_SCANCODE_ESCAPE))
				ctx.close();

			double render_time = ctx.getElapsedTime() - t;
			if (render_time < target_frame_time)
				ctx.sleep(target_frame_time - render_time);

			if (checkGLErrors())
				ctx.close();
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