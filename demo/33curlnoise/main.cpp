#include "app.h"
#include <app/log.h>

void sort()
{
	for (int i = 2; i < 8; i *= 2)
	{
		for (int j = i / 2; j > 0; j /= 2)
		{
			for (int n = 0; n < 8; ++n)
			{
				std::cout << n << " " << pow(n, j) << "\n";
			}
		}
	}
}

int main(int argc, char **argv)
{
	sort();

	GLContext ctx;
	if (!ctx.create(VideoMode(720, 480, 24, 0, 4, 4, 3), "Curlnoise", true, true))
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
		int updates_per_sec = 30;
		double tickrate = 1.0 / updates_per_sec;
		double accumulator = 0.0;
		double update_time = 0.0;
		double render_time = 0.0;
		int num_updates_total = 0;
		int num_frames_total = 0;
		while (ctx.isOpen())
		{
			double frame_t = ctx.getElapsedTime();
			accumulator += dt;
			int num_updates = 0;

			double now = ctx.getElapsedTime();
			while (accumulator >= tickrate && num_updates < 1)
			{
				accumulator -= tickrate;
				update(gfx, ctx, tickrate);
				update_time += ctx.getElapsedTime() - now;
				num_updates++;
				num_updates_total++;
			}
			now = ctx.getElapsedTime();

			render(gfx, ctx, dt);

			num_frames_total++;
			render_time += ctx.getElapsedTime() - now;

			ctx.display();
			ctx.pollEvents();

			if (ctx.isKeyPressed(SDL_SCANCODE_ESCAPE))
				ctx.close();

			if (checkGLErrors())
				ctx.close();
			dt = ctx.getElapsedTime() - frame_t;

			if (num_frames_total > 60)
			{
				num_frames_total = 1;
				num_updates_total = 1;
				update_time = 0.0;
				render_time = 0.0;
			}

			printf("\rupdate: %.2f\trender: %.2f", update_time * 1000.0 / num_updates_total, render_time * 1000.0 / num_frames_total);
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