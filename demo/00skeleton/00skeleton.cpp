#include <iostream>
#include <fstream>
#include <vector>

#include <common/helpers.h>
#include <common/text.h>
#include <common/timer.h>
#include <common/vec.h>
#include <common/matrix.h>
#include <common/matrixstack.h>

#include <gl/opengl.h>
#include <gl/texture.h>
#include <gl/program.h>
#include <gl/bufferobject.h>
#include <gl/vertexformat.h>
#include <gl/bufferedmesh.h>
#include <graphics/spritebatch.h>
#include <graphics/trimesh.h>
#include <graphics/color.h>
#include <graphics/renderer.h>
#include <app/glcontext.h>

bool load()
{
	return true;
}

void free()
{
	
}

void init(Renderer &gfx, Context &ctx)
{

}

void update(Renderer &gfx, Context &ctx, double dt)
{

}

void render(Renderer &gfx, Context &ctx, double dt)
{

}

int main()
{
	GLContext ctx;
	if (!ctx.create(VideoMode(720, 480, 0, 0, 4, 3, 1, false), "Skeleton", true, true))
		return -1;

	Renderer gfx;
	gfx.init(ctx);
	gfx.setClearColor(Color(0.55f, 0.45f, 0.45f, 1.0f));

	if (!load())
	{
		ctx.dispose();
		crash("Failed to load content");
	}

	try
	{
		init(gfx, ctx);

		while(ctx.isOpen())
		{
			gfx.clearColorBuffer();
			ctx.display();
			ctx.pollEvents();
			if (checkGLErrors(std::cerr))
				ctx.close();
		}
	}
	catch (std::exception &e)
	{
		std::cerr << "An unexpected error occured: " << e.what() << std::endl;
	}

	gfx.dispose();
	ctx.dispose();
	shutdown();
}