#include "app.h"
const int   GRID_SIZE	= 50;
const float DX			= 1.0f / GRID_SIZE;
const float WAVE_C		= 0.5f * 0.5f;
const float DISSIPATION = 0.994f;

struct Slab
{
	RenderTexture targets[2];
	int input_index;
	int output_index;

	Slab() : input_index(0), output_index(1) { }
	void swapTargets()
	{
		input_index = output_index;
		output_index = (output_index + 1) % 2;
	}

	RenderTexture &ping() { return targets[input_index]; }
	RenderTexture &pong() { return targets[output_index]; }
};

VertexArray vao;
ShaderProgram 
	shader_mesh,
	shader_poisson,
	shader_boundary;
MeshBuffer water_mesh;
BufferObject 
	vbo_quad,
	vbo_line;

Slab datafield; // R - height, G - velocity

mat4 
	mat_view,
	mat_model,
	mat_projection;

bool load()
{
	if (!shader_mesh.loadFromFile("./demo/10wave2d/simple.vs", "./demo/10wave2d/simple.fs") ||
		!shader_boundary.loadFromFile("./demo/10wave2d/quad.vs", "./demo/10wave2d/boundary.fs") ||
		!shader_poisson.loadFromFile("./demo/10wave2d/quad.vs", "./demo/10wave2d/poisson.fs"))
		return false;

	if (!shader_mesh.linkAndCheckStatus() ||
		!shader_boundary.linkAndCheckStatus() ||
		!shader_poisson.linkAndCheckStatus())
		return false;

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

	// Initial and boundary conditions
	vec2 *data0 = new vec2[GRID_SIZE * GRID_SIZE];
	for (int j = 0; j < GRID_SIZE; ++j)
	{
		for (int i = 0; i < GRID_SIZE; ++i)
		{
			float x = (float)i / (GRID_SIZE - 1);
			float z = (float)j / (GRID_SIZE - 1);
			float r = (x - 0.5f) * (x - 0.5f) + (z - 0.5f) * (z - 0.5f);

			int index = j * GRID_SIZE + i;
			data0[index].x = 0.3f * exp(-10.5f * r); // Height
			data0[index].y = -0.5f * exp(-10.0f * r); // Velocity
		}
	}

	Mesh mesh;
	for (int y = 0; y < GRID_SIZE; ++y)
	{
		for (int x = 0; x < GRID_SIZE; ++x)
		{
			float x0 = -0.5f + x * DX;
			float x1 = -0.5f + (x + 1) * DX;
			float z0 = -0.5f + y * DX;
			float z1 = -0.5f + (y + 1) * DX;
			float positions[] = {
				x0, 0.0f, z0,
				x1, 0.0f, z0,
				x1, 0.0f, z1,
				x0, 0.0f, z1
			};
			float texels[] = {
				x * DX, y * DX,
				(x + 1) * DX, y * DX,
				(x + 1) * DX, (y + 1) * DX,
				x * DX, (y + 1) * DX
			};
			int i = mesh.getPositionCount();
			uint32 indices[] = { i + 0, i + 1, i + 2, i + 2, i + 3, i + 0 };
			mesh.addPositions((vec3*)positions, 4);
			mesh.addTexels((vec2*)texels, 4);
			mesh.addIndices(indices, 6);
		}
	}

	water_mesh.create(mesh);
	water_mesh.bind();

	// Quad that covers the screen except a one pixel boundary
	const float TWODX = 2.0f * DX;
	float quad[] = {
		-1.0f + TWODX, -1.0f + TWODX, 0.0f + DX, 0.0f + DX,
		 1.0f - TWODX, -1.0f + TWODX, 1.0f - DX, 0.0f + DX,
		 1.0f - TWODX,  1.0f - TWODX, 1.0f - DX, 1.0f - DX,
		 1.0f - TWODX,  1.0f - TWODX, 1.0f - DX, 1.0f - DX,
		-1.0f + TWODX,  1.0f - TWODX, 0.0f + DX, 1.0f - DX,
		-1.0f + TWODX, -1.0f + TWODX, 0.0f + DX, 0.0f + DX
	};

	// Lines that cover the boundary
	float line[] = { 
		-1.0f,  1.0f, 0.0f, 1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,  1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, -1.0f,  1.0f, 0.0f, 1.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,  1.0f,  1.0f, 1.0f, 1.0f
	};

	vbo_quad.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(quad), quad);
	vbo_line.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(line), line);

	datafield.ping().create(0, GL_RG32F, GRID_SIZE, GRID_SIZE, GL_RG, GL_FLOAT, data0);
	datafield.pong().create(0, GL_RG32F, GRID_SIZE, GRID_SIZE, GL_RG, GL_FLOAT, data0);
	datafield.ping().getColorBuffer().setTexParameteri(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	datafield.pong().getColorBuffer().setTexParameteri(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	delete[] data0;

	mat_model = transform::scale(2.0f);
	mat_projection = glm::perspective(45.0f, (float)ctx.getWidth() / ctx.getHeight(), 0.1f, 10.0f);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	float mx = (float)ctx.getMouseX() / ctx.getWidth();
	float my = (float)ctx.getMouseY() / ctx.getHeight();

	glViewport(0, 0, GRID_SIZE, GRID_SIZE);
	datafield.pong().begin();
	gfx.beginCustomShader(shader_poisson);
	gfx.setRasterizerState(RasterizerStates::FillBoth);
	vbo_quad.bind();
	datafield.ping().bindTexture();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.setUniform("dt", dt);
	gfx.setUniform("dx", DX);
	gfx.setUniform("dissipation", DISSIPATION);
	gfx.setUniform("wave_c", WAVE_C);
	gfx.setUniform("tex_data", 0);
	gfx.setUniform("mpos", vec2(mx, my));
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);

	gfx.beginCustomShader(shader_boundary);
	vbo_line.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_LINES, 4);

	datafield.pong().end();

	datafield.swapTargets();
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	glViewport(0, 0, ctx.getWidth(), ctx.getHeight());
	mat_view = 
		transform::translate(0.0f, 0.0f, -1.5f) * 
		transform::rotateX(-0.43f) * 
		transform::rotateY(ctx.getElapsedTime() * 0.1f);

	datafield.pong().bindTexture();
	gfx.setRasterizerState(RasterizerStates::LineBoth);
	gfx.beginCustomShader(shader_mesh);
	gfx.setClearColor(0.0f, 0.0f, 0.0f);
	gfx.clearColorBuffer();
	gfx.setUniform("model", mat_model);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("heightmap", 0);
	water_mesh.draw();
}