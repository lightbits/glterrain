#include "app.h"
const int   GRID_SIZE	= 128;
const float DX			= 1.0f / GRID_SIZE;
const float WAVE_C		= 0.5f * 0.5f;
const float DISSIPATION = 0.994f;
const float Z_NEAR      = 0.1f;
const float Z_FAR	    = 20.0f;

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
	shader_heightmap,
	shader_cubemap,
	shader_poisson,
	shader_boundary;
MeshBuffer water_mesh;
MeshBuffer skybox_mesh;
BufferObject 
	vbo_quad,
	vbo_line;

Cubemap cubemap_skybox;

Slab datafield; // R - height, G - velocity

mat4 
	mat_view,
	mat_model,
	mat_projection;

bool load()
{
	if (!shader_heightmap.loadFromFile("./demo/10wave2d/heightmap.vs", "./demo/10wave2d/heightmap.fs") ||
		!shader_cubemap.loadFromFile("./demo/10wave2d/cubemap.vs", "./demo/10wave2d/cubemap.fs") ||
		!shader_boundary.loadFromFile("./demo/10wave2d/quad.vs", "./demo/10wave2d/boundary.fs") ||
		!shader_poisson.loadFromFile("./demo/10wave2d/quad.vs", "./demo/10wave2d/poisson.fs"))
		return false;

	if (!shader_heightmap.linkAndCheckStatus() ||
		!shader_cubemap.linkAndCheckStatus() ||
		!shader_boundary.linkAndCheckStatus() ||
		!shader_poisson.linkAndCheckStatus())
		return false;

	if (!cubemap_skybox.loadFromFile("./data/cubemaps/iceland_", ".jpg"))
		return false;

	cubemap_skybox.setTexParameteri(
		GL_LINEAR, 
		GL_LINEAR, 
		GL_CLAMP_TO_EDGE, 
		GL_CLAMP_TO_EDGE, 
		GL_CLAMP_TO_EDGE);

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
			if (i > 0 && i < GRID_SIZE - 1 && j > 0 && j < GRID_SIZE - 1)
			{
				data0[index].x = 0.3f * exp(-10.5f * r); // Height
				data0[index].y = -0.5f * exp(-10.0f * r); // Velocity
			}
			else
			{
				data0[index].x = 0.0f;
				data0[index].y = 0.0f;
			}
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
			uint32 indices[] = { i + 0, i + 3, i + 2, i + 2, i + 1, i + 0 };
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

	skybox_mesh.create(Mesh::genUnitCube(false, false, false));

	mat_view = mat4(1.0f);
	mat_model = transform::scale(1.0f);
	mat_projection = glm::perspective(PI / 4.0f, (float)ctx.getWidth() / ctx.getHeight(), Z_NEAR, Z_FAR);
}

// Casts a ray from the window coordinates onto the water plane
vec3 raycast(int x, int y, int w, int h)
{
	float xndc = -1.0f + 2.0f * x / w;
	float yndc =  1.0f - 2.0f * y / h;

	vec4 view = glm::inverse(mat_projection) * vec4(xndc, yndc, 1.0f, 1.0f);

	vec3 origin = vec3(glm::inverse(mat_view) * vec4(0.0f, 0.0f, 0.0f, 1.0f));
	vec3 dir = glm::normalize(vec3(glm::inverse(mat_view) * view));
	float t = -origin.y / dir.y;

	return origin + t * dir;
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	mat_view = 
		transform::translate(0.0f, 0.0f, -1.5f) * 
		transform::rotateX(-0.4f) * 
		transform::rotateY(ctx.getElapsedTime() * 0.1f);

	vec3 p = raycast(ctx.getMouseX(), ctx.getMouseY(), ctx.getWidth(), ctx.getHeight());

	glViewport(0, 0, GRID_SIZE, GRID_SIZE);
	datafield.pong().begin();
	gfx.beginCustomShader(shader_poisson);
	gfx.setDepthTestState(DepthTestStates::Disabled);
	gfx.setRasterizerState(RasterizerStates::FillBoth); // This is important!
	vbo_quad.bind();
	datafield.ping().bindTexture();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.setUniform("dt", dt);
	gfx.setUniform("dx", DX);
	gfx.setUniform("dissipation", DISSIPATION);
	gfx.setUniform("wave_c", WAVE_C);
	gfx.setUniform("tex_data", 0);
	gfx.setUniform("mpos", vec2(p.x + 0.5f, p.z + 0.5f));
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
	gfx.setClearColor(0.0f, 0.0f, 0.0f);
	gfx.clearColorBuffer();

	// Cubemap
	gfx.setRasterizerState(RasterizerStates::FillBoth);
	gfx.setDepthTestState(DepthTestStates::Disabled);
	gfx.beginCustomShader(shader_cubemap);
	cubemap_skybox.bind();
	gfx.setUniform("model", transform::scale(10.0f)); // Scale up the cube
	gfx.setUniform("view", mat_view);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("cubemap", 0);
	skybox_mesh.draw();

	// Heightmap
	glActiveTexture(GL_TEXTURE0);
	datafield.pong().bindTexture();
	glActiveTexture(GL_TEXTURE1);
	cubemap_skybox.bind();
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setClearDepth(1.0);
	gfx.clearDepthBuffer();
	gfx.setRasterizerState(RasterizerStates::FillBoth);
	gfx.beginCustomShader(shader_heightmap);
	gfx.setUniform("model", mat_model);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("dx", DX);
	gfx.setUniform("heightmap", 0);
	gfx.setUniform("cubemap", 1);
	water_mesh.draw();
}