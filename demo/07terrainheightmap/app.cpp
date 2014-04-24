#include "app.h"
#include <common/transform.h>

Texture2D 
	tex_height,
	tex_normal,
	tex_diffuse;

ShaderProgram
	shader_terrain,
	shader_normals;

/* Terrain params */
Mesh mesh_terrain;
MeshBuffer buffer_terrain;
BufferObject normals_vbo;
int terrain_res_x;
int terrain_res_y;
float terrain_height;

mat4
	mat_model,
	mat_view,
	mat_projection;

bool load()
{
	if (!shader_terrain.loadAndLinkFromFile("./demo/07terrainheightmap/terrain") ||
		!shader_normals.loadAndLinkFromFile("./demo/07terrainheightmap/normals"))
		return false;

	if (!tex_height.loadFromFile("./demo/07terrainheightmap/terrainHeight.png") ||
		!tex_normal.loadFromFile("./demo/07terrainheightmap/terrainNormal.png") ||
		!tex_diffuse.loadFromFile("./demo/07terrainheightmap/terrainDiffuse.png"))
		return false;

	tex_height.setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	return true;
}

void free()
{
	shader_terrain.dispose();
	shader_normals.dispose();
	tex_height.dispose();
	tex_normal.dispose();
	tex_diffuse.dispose();
}

void init(Renderer &gfx, Context &ctx)
{
	terrain_res_x = 64;
	terrain_res_y = 64;
	normals_vbo.create(
		GL_ARRAY_BUFFER, 
		GL_STATIC_DRAW, 
		10 * sizeof(float) * terrain_res_x * terrain_res_y, 
		NULL);
	normals_vbo.bind();
	float dx = 1.0f / terrain_res_x;
	float dy = 1.0f / terrain_res_y;
	for (int y = 0; y < terrain_res_y; ++y)
	{
		for (int x = 0; x < terrain_res_x; ++x)
		{
			float u0 = x * dx;
			float u1 = (x + 1) * dx;
			float v0 = y * dy;
			float v1 = (y + 1) * dy;

			float x0 = -0.5f + u0;
			float x1 = -0.5f + u1;
			float z0 = -0.5f + v0;
			float z1 = -0.5f + v1;
			const float vertices[] = {
				x0, 0.0f, z0,
				x1, 0.0f, z0,
				x1, 0.0f, z1,
				x0, 0.0f, z1
			};
			const float texels[] = {
				u0, v0,
				u1, v0,
				u1, v1,
				u0, v1
			};
			uint32 i = mesh_terrain.getPositionCount(); 
			mesh_terrain.addPositions((vec3*)vertices, 4);
			mesh_terrain.addTexels((vec2*)texels, 4);
			mesh_terrain.addTriangle(i, i + 3, i + 2);
			mesh_terrain.addTriangle(i + 2, i + 1, i);

			// for debugging purposes we render the normals
			const float lines[] = { 
				x0, 0.0f, z0, u0, v0, x0, 1.0f, z0, u0, v0
			};
			normals_vbo.bufferSubData(
				10 * sizeof(float) * (y * terrain_res_x + x), 
				sizeof(lines), 
				lines);
		}
	}
	normals_vbo.unbind();
	buffer_terrain = MeshBuffer(mesh_terrain);

	mat_model = mat4(1.0f);
	mat_view = mat4(1.0f);
	mat_projection = glm::perspective(45.0f, (float)ctx.getWidth() / ctx.getHeight(), 0.1f, 10.0f);

	terrain_height = 0.25f;
}

void update(Renderer &gfx, Context &ctx, float dt)
{
	mat_model = transform::scale(2.0f);

	mat_view = 
		transform::translate(0.0f, -0.2f, -0.8f) *
		transform::rotateX(-0.5f) *
		transform::rotateY(ctx.getElapsedTime() * 0.2f);

	if (ctx.isKeyPressed('w'))
		terrain_height += dt;
	else if (ctx.isKeyPressed('s'))
		terrain_height -= dt;
}

void render(Renderer &gfx, Context &ctx, float dt)
{
	gfx.setRasterizerState(RasterizerStates::LineBoth);
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setClearColor(0.71f, 0.68f, 0.68f);
	gfx.setClearDepth(1.0);
	gfx.clearColorAndDepth();

	gfx.beginCustomShader(shader_terrain);
	tex_height.bind(GL_TEXTURE0);
	tex_normal.bind(GL_TEXTURE1);
	tex_diffuse.bind(GL_TEXTURE2);
	gfx.setUniform("tex_height", 0);
	gfx.setUniform("tex_normal", 1);
	gfx.setUniform("tex_diffuse", 2);
	gfx.setUniform("terrain_height", terrain_height);
	gfx.setUniform("terrain_res_x", terrain_res_x);
	gfx.setUniform("terrain_res_y", terrain_res_y);
	gfx.setUniform("model", mat_model);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("projection", mat_projection);
	buffer_terrain.draw();
	tex_height.unbind();
	gfx.endCustomShader();

	// Render normals
	//gfx.beginCustomShader(shader_normals);
	//tex_height.bind(GL_TEXTURE0);
	//tex_normal.bind(GL_TEXTURE1);
	//gfx.setUniform("tex_height", 0);
	//gfx.setUniform("tex_normal", 1);
	//gfx.setUniform("terrain_res_x", terrain_res_x);
	//gfx.setUniform("terrain_res_y", terrain_res_y);
	//gfx.setUniform("terrain_height", terrain_height);
	//gfx.setUniform("model", mat_model);
	//gfx.setUniform("view", mat_view);
	//gfx.setUniform("projection", mat_projection);
	//normals_vbo.bind();
	//gfx.setAttributefv("position", 3, 5, 0);
	//gfx.setAttributefv("texel", 2, 5, 3);
	//gfx.drawVertexBuffer(GL_LINES, 2 * terrain_res_x * terrain_res_y);
	//normals_vbo.unbind();
	//gfx.endCustomShader();
}