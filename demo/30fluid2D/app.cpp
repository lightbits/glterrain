#include "app.h"
#include "slab.h"
#include "slabops.h"
#include <common/text.h>
float	amplification = 1.0f;
int		display_mode = 0;

VertexArray vao;
ShaderProgram 
	shader_texture,
	shader_vector;

BufferObject 
	vbo_quad,
	vbo_vectors;

Slab velocity, pressure, dye, divergence;
SpriteBatch spritebatch;
Font font;

bool load()
{
	if (!loadSlabOps())
		return false;

	if (!shader_vector.loadFromFile("./demo/30fluid2D/vector.vs", "./demo/30fluid2D/vector.fs") ||
		!shader_texture.loadFromFile("./demo/30fluid2D/quad.vs", "./demo/30fluid2D/texture.fs"))
		return false;

	if (!shader_vector.linkAndCheckStatus() ||
		!shader_texture.linkAndCheckStatus())
		return false;

	if (!font.loadFromFile("./data/fonts/proggytinyttsz_8x12.png"))
		return false;

	spritebatch.create();
	spritebatch.setFont(font);

	return true;
}

void free()
{
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();

	velocity.createSurfaces(GRID_SIZE, GRID_SIZE, 2, NULL);
	pressure.createSurfaces(GRID_SIZE, GRID_SIZE, 2, NULL);
	dye.createSurfaces(GRID_SIZE, GRID_SIZE, 2, NULL);
	divergence.createSurfaces(GRID_SIZE, GRID_SIZE, 2, NULL);

	float *vectors = new float[GRID_SIZE * GRID_SIZE * 2 * 5 / 64];
	int index = 0;
	for (int j = 0; j < GRID_SIZE; j += 8)
	{
		for (int i = 0; i < GRID_SIZE; i += 8)
		{
			float x = -1.0f + 2.0f * i / GRID_SIZE;
			float y = -1.0f + 2.0f * j / GRID_SIZE;
			float u = (float)i / GRID_SIZE;
			float v = (float)j / GRID_SIZE;
			vectors[index++] = x;
			vectors[index++] = y;
			vectors[index++] = u;
			vectors[index++] = v;
			vectors[index++] = 0.0f;
			vectors[index++] = x;
			vectors[index++] = y;
			vectors[index++] = u;
			vectors[index++] = v;
			vectors[index++] = 1.0f;
		}
	}
	vbo_vectors.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, GRID_SIZE * GRID_SIZE * 2 * 5 / 64 * sizeof(float), vectors);
	delete[] vectors;

	float quad[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f
	};
	vbo_quad.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(quad), quad);
}

vec2i last_mouse_pos;
void handleMouse(Renderer &gfx, Context &ctx, float dt)
{
	vec2i mouse_pos = ctx.getMousePos();
	vec2 p = vec2(mouse_pos.x, ctx.getHeight() - mouse_pos.y);
	p.x /= (float)ctx.getWidth();
	p.y /= (float)ctx.getHeight();

	vec2 v = mouse_pos - last_mouse_pos;
	v.x /= (float)ctx.getWidth();
	v.y /= (float)ctx.getHeight();
	v.y *= -1.0f; // Because the mouse y flipped
	last_mouse_pos = mouse_pos;

	if (ctx.isMousePressed(SDL_BUTTON_LEFT))
	{
		addDye(dye.ping(), dye.pong(), p, gfx, ctx, dt);
		dye.swapSurfaces();

		addForces(velocity.ping(), velocity.pong(), p, v, gfx, ctx, dt);
		velocity.swapSurfaces();
	}
}

void update(Renderer &gfx, Context &ctx, float dt)
{
	glViewport(0, 0, GRID_SIZE, GRID_SIZE);

	advect(velocity.ping(), dye.ping(), dye.pong(), 0.97f, gfx, ctx, dt);
	dye.swapSurfaces();

	advect(velocity.ping(), velocity.ping(), velocity.pong(), 0.99f, gfx, ctx, dt);
	velocity.swapSurfaces();

	handleMouse(gfx, ctx, dt);

	calculateDivergence(velocity.ping(), divergence.pong(), gfx, ctx, dt);

	// Clear pressure field (initial guess is 0)
	pressure.ping().begin();
	gfx.setClearColor(0.0f, 0.0f, 0.0f);
	gfx.clearColorAndDepth();
	pressure.ping().end();

	// Calculate pressure field
	for (int i = 0; i < 200; ++i)
	{
		jacobi(pressure.ping(), pressure.pong(), divergence.pong(), -(DX * DX), 4.0f, gfx, ctx, dt);
		pressure.swapSurfaces();
	}

	subtractGradient(velocity.ping(), pressure.pong(), velocity.pong(), gfx, ctx, dt);
	velocity.swapSurfaces();

	// These don't work!
	//applyBoundaryConditions(velocity.ping(), velocity.pong(), -1.0f, gfx, ctx, dt);
	//velocity.swapSurfaces();

	//applyBoundaryConditions(pressure.ping(), pressure.pong(), 1.0f, gfx, ctx, dt);
	//pressure.swapSurfaces();
}

void drawVelocity(Renderer &gfx, Context &ctx, float dt)
{
	vbo_quad.bind();
	velocity.ping().bindTexture(GL_TEXTURE0);
	gfx.beginCustomShader(shader_texture);
	gfx.setUniform("tex", 0);
	gfx.setUniform("amplification", amplification);
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);

	vbo_vectors.bind();
	velocity.ping().bindTexture(GL_TEXTURE0);
	gfx.beginCustomShader(shader_vector);
	gfx.setUniform("tex_velocity", 0);
	gfx.setAttributefv("position", 2, 5, 0);
	gfx.setAttributefv("texel", 2, 5, 2);
	gfx.setAttributefv("scale", 2, 5, 4);
	gfx.drawVertexBuffer(GL_LINES, GRID_SIZE * GRID_SIZE * 2 / 64);
}

void drawDye(Renderer &gfx, Context &ctx, float dt)
{
	vbo_quad.bind();
	dye.ping().bindTexture(GL_TEXTURE0);
	gfx.beginCustomShader(shader_texture);
	gfx.setClearColor(0.0f, 0.0f, 0.0f);
	gfx.clearColorAndDepth();
	gfx.setUniform("tex", 0);
	gfx.setUniform("amplification", amplification);
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
}

void drawPressure(Renderer &gfx, Context &ctx, float dt)
{
	vbo_quad.bind();
	pressure.ping().bindTexture(GL_TEXTURE0);
	gfx.beginCustomShader(shader_texture);
	gfx.setUniform("tex", 0);
	gfx.setUniform("amplification", amplification);
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
}

void render(Renderer &gfx, Context &ctx, float dt)
{
	glViewport(0, 0, ctx.getWidth(), ctx.getHeight());
	gfx.setClearColor(0.0f, 0.0f, 0.0f);
	gfx.clearColorAndDepth();

	string display_string = "";
	switch (display_mode)
	{
	case 0:
		drawVelocity(gfx, ctx, dt);
		display_string = "Velocity field";
		break;
	case 1:
		drawDye(gfx, ctx, dt);
		display_string = "Dye advection";
		break;
	case 2:
		drawPressure(gfx, ctx, dt);
		display_string = "Pressure field";
		break;
	}

	spritebatch.begin();
	spritebatch.drawQuad(
		Color::fromHex(0x00000099), 
		Rectanglef(0.0f, 0.0f, ctx.getWidth(), 21.0f));

	spritebatch.drawString(
		"Press SPACE to change display mode", 
		vec2(5.0f, 5.0f), 
		Colors::White
		);

	Text amp;
	amp << "Amplification " << amplification;
	spritebatch.drawString(
		amp.getString(), 
		vec2(ctx.getWidth() - 5.0f - font.measureString(amp.getString()), 5.0f), 
		Colors::White
		);

	spritebatch.drawString(
		display_string, 
		vec2(ctx.getWidth() / 2 - font.measureString(display_string) / 2, 5.0f),
		Colors::White
		);

	spritebatch.end();
}

void keyPressed(int mod, SDL_Keycode key)
{
	switch (key)
	{
	case SDLK_SPACE:
		display_mode = (display_mode + 1) % 3;
		break;
	case SDLK_a:
		amplification *= 0.5f;
		break;
	case SDLK_s:
		amplification *= 2.0f;
		break;
	}
}