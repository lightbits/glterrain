#include "app.h"
#include <app/log.h>

int main(int argc, char **argv)
{
	Log log("log.txt", true);
	GLContext ctx;
	if (!ctx.create(VideoMode(720, 480, 24, 0, 4), "04 Vertex Array Object", true, true))
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

		double dt = 0.0;
		while (ctx.isOpen())
		{
			double frame_t = ctx.getElapsedTime();
			update(gfx, ctx, dt);
			
			render(gfx, ctx, dt);
			ctx.display();
			ctx.pollEvents();

			if (ctx.isKeyPressed(SDL_SCANCODE_ESCAPE))
				ctx.close();

			if (checkGLErrors(log))
				ctx.close();
			dt = ctx.getElapsedTime() - frame_t;
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