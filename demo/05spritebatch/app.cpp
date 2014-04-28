/*
Uses vertex array objects to store vertex buffer bindings and attribute pointer thingies.
*/

#include "app.h"

VertexArray vao;
SpriteBatch spritebatch;
Font font;
Texture2D texture;

bool load()
{
	if (!font.loadFromFile("./data/fonts/proggytinyttsz_8x12.png") ||
		!texture.loadFromFile("./data/textures/tex0.png"))
		return false;
	return true;
}

void free()
{
	vao.dispose();
	spritebatch.dispose();
}

void init(Renderer &gfx, Context &ctx)
{
	// This loads default shaders and does some more work
	spritebatch.create();

	// Default VAO
	vao.create();
	vao.bind();
}

void update(Renderer &gfx, Context &ctx, double dt)
{

}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.setClearDepth(1.0);
	gfx.setClearColor(Color::fromHex(0x333333ff));
	gfx.clearColorAndDepth();

	spritebatch.begin();
	spritebatch.setFont(font);
	spritebatch.drawString(
		"Hello World!", 
		vec2(5.0f, 5.0f), 
		Color::fromHex(0xaaaaffff),
		2.0f);

	spritebatch.drawTexture(
		texture, 
		Colors::White, 
		Rectanglef(300.0f, 300.0f, 300.0f, 300.0f), 
		1.0f, 0.0f,
		ctx.getElapsedTime(), 
		vec2(0.5f, 0.5f));
	spritebatch.end();
}