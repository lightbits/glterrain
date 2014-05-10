#include "app.h"

VertexArray vao;
ShaderProgram shader;

const int GRID_SIZE = 40;
BufferObject vbo;
float *U_prev; // Heightfield
float *U_curr;
float *V_prev; // Velocity field
float *V_curr;

void swap(float **a, float **b)
{
	float *temp = *a;
	*a = *b;
	*b = temp;
}

void uploadMesh()
{
	vec2 vertices[GRID_SIZE * 6];
	const float dx = 2.0f / GRID_SIZE;
	for (int i = 0; i < GRID_SIZE; ++i)
	{
		int j = i * 6;
		float y0 = U_curr[i];
		float y1 = U_curr[i];
		if (i < GRID_SIZE - 1)
			y1 = U_curr[i + 1];
		float x = -1.0f + i * dx;
		vertices[j + 0] = vec2(x - 0.5 * dx, -1.0f);
		vertices[j + 1] = vec2(x + 0.5 * dx, -1.0f);
		vertices[j + 2] = vec2(x + 0.5 * dx, y1);
		vertices[j + 3] = vec2(x + 0.5 * dx, y1);
		vertices[j + 4] = vec2(x - 0.5 * dx, y0);
		vertices[j + 5] = vec2(x - 0.5 * dx, -1.0f);
	}
	vbo.bufferSubData(0, sizeof(vec2) * GRID_SIZE * 6, &vertices[0]);
}

bool load()
{
	if (!shader.loadAndLinkFromFile("./demo/09wave1d/simple"))
		return false;

	return true;
}

void free()
{
	delete[] U_prev;
	delete[] U_curr;
	delete[] V_prev;
	delete[] V_curr;
	vbo.dispose();
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{	
	vao.create();
	vao.bind();

	U_prev = new float[GRID_SIZE];
	U_curr = new float[GRID_SIZE];
	V_prev = new float[GRID_SIZE];
	V_curr = new float[GRID_SIZE];

	// Initial and boundary conditions
	for (int i = 0; i < GRID_SIZE; ++i)
	{
		float x = (float)i / GRID_SIZE;
		float y = sin(4.0f * PI * x) * 0.2f;
		U_curr[i] = y;
		U_prev[i] = U_curr[i];
		V_curr[i] = cos(PI * x) * 0.2f;
		V_prev[i] = V_curr[i];
	}

	U_curr[0] = 0.0f;
	U_curr[GRID_SIZE - 1] = 0.0f;
	U_prev[0] = U_curr[0];
	U_prev[GRID_SIZE - 1] = U_curr[GRID_SIZE - 1];

	vbo.create(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, sizeof(vec2) * GRID_SIZE * 6, NULL);
	vbo.bind();
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	if (ctx.isMousePressed(SDL_BUTTON_LEFT))
	{
		float x = (float)ctx.getMouseX() / ctx.getWidth();
		float y = 1.0f - 2.0f * ctx.getMouseY() / ctx.getHeight();
		U_prev[(int)(x * (GRID_SIZE - 1))] = y;
		V_prev[(int)(x * (GRID_SIZE - 1))] = 0.0f;
	}

	const float dx = 1.0f / GRID_SIZE;
	const float c = 0.5f;
	for (int i = 1; i < GRID_SIZE - 1; ++i)
	{
		V_curr[i] = 0.994f * V_prev[i];
		float ur = U_prev[i + 1];
		float ul = U_prev[i - 1];
		float uc = U_prev[i];
		V_curr[i] += (dt * c * c) * (ur + ul - 2.0f * uc) / (dx * dx);
		U_curr[i] = U_prev[i];
		U_curr[i] += dt * V_curr[i];
	}

	U_curr[0] = 0.0f;
	U_curr[GRID_SIZE - 1] = 0.0f;
	//V_curr[

	uploadMesh();

	swap(&V_curr, &V_prev);
	swap(&U_curr, &U_prev);
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.beginCustomShader(shader);
	gfx.setClearColor(0.0f, 0.0f, 0.0f);
	gfx.clearColorBuffer();
	gfx.setAttributefv("position", 2, 0, 0);
	gfx.drawVertexBuffer(GL_TRIANGLES, GRID_SIZE * 6);
}