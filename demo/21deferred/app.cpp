#include "app.h"

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
	gfx.setClearColor(0.2f, 0.2f, 0.3f);
	gfx.clearColorBuffer();
}