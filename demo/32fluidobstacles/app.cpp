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

BufferObject vbo_quad;

Slab 
	velocity, 
	pressure,
	density,
	divergence, 
	obstacles,
	temperature;
SpriteBatch spritebatch;
Font font;

bool load()
{
	if (!loadSlabOps())
		return false;

	if (!shader_vector.loadFromFile("./demo/32fluidobstacles/vector.vs", "./demo/32fluidobstacles/vector.fs") ||
		!shader_texture.loadFromFile("./demo/32fluidobstacles/quad.vs", "./demo/32fluidobstacles/texture.fs"))
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

	velocity.createSurfaces(GRID_SIZE, GRID_SIZE, 4, NULL);
	pressure.createSurfaces(GRID_SIZE, GRID_SIZE, 4, NULL);
	density.createSurfaces(GRID_SIZE, GRID_SIZE, 4, NULL);
	divergence.createSurfaces(GRID_SIZE, GRID_SIZE, 4, NULL);
	temperature.createSurfaces(GRID_SIZE, GRID_SIZE, 4, NULL);

	vec4 *init_obstacles = new vec4[GRID_SIZE * GRID_SIZE];
	for (int j = 0; j < GRID_SIZE; ++j)
	{
		for (int i = 0; i < GRID_SIZE; ++i)
		{
			float x = (float)i / GRID_SIZE;
			float y = (float)j / GRID_SIZE;
			float dx = x - 0.5f;
			float dy = y - 0.5f;
			float r = (dx * dx * 2.0f) + dy * dy;
			if (r < 0.15f * 0.15f)
				init_obstacles[j * GRID_SIZE + i] = vec4(0.0f);
			else
				init_obstacles[j * GRID_SIZE + i] = vec4(1.0f);
		}
	}
	obstacles.createSurfaces(GRID_SIZE, GRID_SIZE, 4, (const float*)init_obstacles);
	delete[] init_obstacles;

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

void update(Renderer &gfx, Context &ctx, float dt)
{
	glViewport(0, 0, GRID_SIZE, GRID_SIZE);

	advect(velocity.ping(), velocity.ping(), velocity.pong(), obstacles.ping(), 0.99f, gfx, ctx, dt);
	velocity.swapSurfaces();

	advect(velocity.ping(), density.ping(), density.pong(), obstacles.ping(), 0.99f, gfx, ctx, dt);
	density.swapSurfaces();

	//advect(velocity.ping(), temperature.ping(), temperature.pong(), obstacles.ping(), 0.99f, gfx, ctx, dt);
	//temperature.swapSurfaces();

	addBuocancy(velocity.ping(), temperature.ping(), velocity.pong(), gfx, ctx, dt);
	velocity.swapSurfaces();

	applyImpulse(density.ping(), density.pong(), vec2(0.5f, 0.0f), 0.02f, 10.0f, gfx, ctx, dt);
	applyImpulse(temperature.ping(), temperature.pong(), vec2(0.5f, 0.0f), 0.1f, 1.0f, gfx, ctx, dt);
	density.swapSurfaces();
	temperature.swapSurfaces();

	calculateDivergence(velocity.ping(), divergence.pong(), obstacles.ping(), gfx, ctx, dt);

	// Clear pressure field (initial guess is 0)
	pressure.ping().begin();
	gfx.setClearColor(0.0f, 0.0f, 0.0f);
	gfx.clearColorAndDepth();
	pressure.ping().end();

	// Calculate pressure field
	for (int i = 0; i < 40; ++i)
	{
		jacobi(pressure.ping(), pressure.pong(), divergence.pong(), obstacles.ping(), -(DX * DX), 4.0f, gfx, ctx, dt);
		pressure.swapSurfaces();
	}

	subtractGradient(velocity.ping(), pressure.pong(), velocity.pong(), obstacles.ping(), gfx, ctx, dt);
	velocity.swapSurfaces();
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
}

void drawDensity(Renderer &gfx, Context &ctx, float dt)
{
	vbo_quad.bind();
	density.ping().bindTexture(GL_TEXTURE0);
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
		drawDensity(gfx, ctx, dt);
		display_string = "Density field";
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