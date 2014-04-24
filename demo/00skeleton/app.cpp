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

void update(Renderer &gfx, Context &ctx, float dt)
{

}

void render(Renderer &gfx, Context &ctx, float dt)
{
	gfx.setClearColor(0.2f, 0.2f, 0.3f);
	gfx.clearColorBuffer();
}

void keyPressed(int mod, SDL_Keycode key)
{

}

void keyReleased(int mod, SDL_Keycode key)
{
	cout << key << endl;
}

void mouseMoved(int x, int y, int dx, int dy)
{

}

void mouseDragged(int button, int x, int y, int dx, int dy)
{

}

void mousePressed(int button, int x, int y)
{

}

void mouseReleased(int button, int x, int y)
{

}