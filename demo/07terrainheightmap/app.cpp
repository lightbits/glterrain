#include "app.h"
#include <common/transform.h>

Texture 
	tex_height,
	tex_normal,
	tex_diffuse;

ShaderProgram
	shader_terrain;

/* Terrain params */
Mesh mesh_terrain;
MeshBuffer buffer_terrain;
int terrain_res_x;
int terrain_res_y;
float terrain_height;

mat4
	mat_model,
	mat_view,
	mat_projection;

bool load()
{
	if (!shader_terrain.loadFromFile("./demo/07terrainheightmap/terrain"))
		return false;

	if (!shader_terrain.linkAndCheckStatus())
		return false;

	if (!tex_height.loadFromFile("./demo/07terrainheightmap/terrainHeight.png") ||
		!tex_normal.loadFromFile("./demo/07terrainheightmap/terrainNormal.png") ||
		!tex_diffuse.loadFromFile("./demo/07terrainheightmap/terrainDiffuse.png"))
		return false;

	return true;
}

void free()
{
	shader_terrain.dispose();
	tex_height.dispose();
	tex_normal.dispose();
	tex_diffuse.dispose();
}

void init(Renderer &gfx, Context &ctx)
{
	terrain_res_x = 128;
	terrain_res_y = 128;
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
		}
	}

	buffer_terrain = MeshBuffer(mesh_terrain);

	mat_model = mat4(1.0f);
	mat_view = mat4(1.0f);
	mat_projection = glm::perspective(45.0f, (float)ctx.getWidth() / ctx.getHeight(), 0.1f, 10.0f);

	terrain_height = 0.25f;
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	mat_view = 
		transform::translate(0.0f, -0.1f, -0.8f) *
		transform::rotateX(-0.24f) *
		transform::rotateY(ctx.getElapsedTime() * 0.2f);

	if (ctx.isKeyPressed('w'))
		terrain_height += dt;
	else if (ctx.isKeyPressed('s'))
		terrain_height -= dt;
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.setRasterizerState(RasterizerStates::LineBack);
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setClearColor(0.2f, 0.2f, 0.3f);
	gfx.setClearDepth(1.0);
	gfx.clearColorAndDepth();

	gfx.beginCustomShader(shader_terrain);
	glActiveTexture(GL_TEXTURE0);
	tex_height.bind();
	glActiveTexture(GL_TEXTURE1);
	tex_normal.bind();
	glActiveTexture(GL_TEXTURE2);
	tex_diffuse.bind();
	gfx.setUniform("terrain_height", terrain_height);
	gfx.setUniform("tex_height", 0);
	gfx.setUniform("tex_normal", 1);
	gfx.setUniform("tex_diffuse", 2);
	gfx.setUniform("model", mat_model);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("projection", mat_projection);
	buffer_terrain.draw();
	tex_height.unbind();
	gfx.endCustomShader();
}