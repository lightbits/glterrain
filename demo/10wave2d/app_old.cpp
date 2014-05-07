#include "app.h"

struct Field
{
	float *data;
	int width;
	int height;

	void init(int w, int h)
	{
		data = new float[w * h];
		for (int i = 0; i < w * h; ++i)
			data[i] = 0.0f;
		width = w;
		height = h;
	}

	void dispose()
	{
		delete[] data;
	}

	float &operator()(int x, int y)
	{
		return data[y * width + x];
	}
};

VertexArray vao;
ShaderProgram shader;
const int GRID_SIZE = 25;
MeshBuffer vertexBuffer;

Field 
	U_prev, // Heightfield
	U_curr, 
	V_prev, // Velocity field
	V_curr;

mat4 
	mat_view,
	mat_model,
	mat_projection;

Texture2D heightmap; // To upload to vertex shader

void swap(Field &a, Field &b)
{
	float *temp = a.data;
	a.data = b.data;
	b.data = temp;
}

bool load()
{
	if (!shader.loadAndLinkFromFile("./demo/10wave2d/simple"))
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

	U_prev.init(GRID_SIZE, GRID_SIZE);
	U_curr.init(GRID_SIZE, GRID_SIZE);
	V_prev.init(GRID_SIZE, GRID_SIZE);
	V_curr.init(GRID_SIZE, GRID_SIZE);

	// Initial and boundary conditions
	for (int j = 0; j < GRID_SIZE; ++j)
	{
		for (int i = 0; i < GRID_SIZE; ++i)
		{
			float x = (float)i / (GRID_SIZE - 1);
			float z = (float)j / (GRID_SIZE - 1);
			float y = sin(2.0f * PI * x) * 0.2f + 0.2f;
			float r = (x - 0.5f) * (x - 0.5f) + (z - 0.5f) * (z - 0.5f);
			U_curr(i, j) = 0.5f * exp(-10.5f * r);
			U_prev(i, j) = U_curr(i, j);
			V_curr(i, j) = -0.5f * exp(-10.5f * r);
			V_prev(i, j) = V_curr(i, j);
		}
	}

	Mesh mesh;
	float dx = 1.0f / GRID_SIZE;
	for (int y = 0; y < GRID_SIZE; ++y)
	{
		for (int x = 0; x < GRID_SIZE; ++x)
		{
			float x0 = -0.5f + x * dx;
			float x1 = -0.5f + (x + 1) * dx;
			float z0 = -0.5f + y * dx;
			float z1 = -0.5f + (y + 1) * dx;
			float positions[] = {
				x0, 0.0f, z0,
				x1, 0.0f, z0,
				x1, 0.0f, z1,
				x0, 0.0f, z1
			};
			float texels[] = {
				x * dx, y * dx,
				(x + 1) * dx, y * dx,
				(x + 1) * dx, (y + 1) * dx,
				x * dx, (y + 1) * dx
			};
			int i = mesh.getPositionCount();
			uint32 indices[] = { i + 0, i + 1, i + 2, i + 2, i + 3, i + 0 };
			mesh.addPositions((vec3*)positions, 4);
			mesh.addTexels((vec2*)texels, 4);
			mesh.addIndices(indices, 6);
		}
	}

	vertexBuffer.create(mesh);
	vertexBuffer.bind();

	heightmap.create(0, GL_R32F, GRID_SIZE, GRID_SIZE, GL_RED, GL_FLOAT, U_prev.data);
	heightmap.setTexParameteri(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	heightmap.bind();

	mat_model = transform::scale(2.0f);
	mat_projection = glm::perspective(45.0f, (float)ctx.getWidth() / ctx.getHeight(), 0.1f, 10.0f);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	// Step forward in simulation
	const float dx = 1.0f / GRID_SIZE;
	const float c = 0.5f;
	for (int j = 1; j < GRID_SIZE - 1; ++j)
	{
		for (int i = 1; i < GRID_SIZE - 1; ++i)
		{
			V_curr(i, j) = 0.994 * V_prev(i, j); // Attenuation of waves
			float ur = U_prev(i + 1, j);
			float ul = U_prev(i - 1, j);
			float ut = U_prev(i, j + 1);
			float ub = U_prev(i, j - 1);
			float uc = U_prev(i, j);
			V_curr(i, j) += (dt * c * c) * (ur + ul + ut + ub - 4.0f * uc) / (dx * dx);
			U_curr(i, j) = U_prev(i, j) + dt * V_curr(i, j);
		}
	}

	// Boundary conditions
	for (int i = 0; i < GRID_SIZE; ++i)
	{
		U_curr(0, i) = 0.0f;
		U_curr(i, 0) = 0.0f;
		U_curr(GRID_SIZE - 1, i) = 0.0f;
		U_curr(i, GRID_SIZE - 1) = 0.0f;
	}

	swap(U_curr, U_prev);
	swap(V_curr, V_prev);

	// Upload height data to texture
	heightmap.update(U_curr.data, GL_RED, GL_FLOAT);
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	mat_view = 
		transform::translate(0.0f, 0.0f, -1.5f) * 
		transform::rotateX(-0.43f) * 
		transform::rotateY(ctx.getElapsedTime() * 0.3f);

	gfx.setRasterizerState(RasterizerStates::LineBoth);
	gfx.beginCustomShader(shader);
	gfx.setClearColor(0.0f, 0.0f, 0.0f);
	gfx.clearColorBuffer();
	gfx.setUniform("model", mat_model);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("heightmap", 0);
	vertexBuffer.draw();
}