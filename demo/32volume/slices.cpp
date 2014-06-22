/*
Volume rendering techniques
http://http.developer.nvidia.com/GPUGems/gpugems_ch39.html
*/

#include "app.h"
#include <common/noise.h>
#include <common/text.h>
using namespace transform;

VertexArray vao;
ShaderProgram 
	shader_simple, 
	shader_genvolume, 
	shader_slices,
	shader_planes;
MeshBuffer
	buf_quads,
	buf_cube;
mat4 
	mat_projection, 
	mat_view;
GLuint 
	tex_volume;
SpriteBatch 
	spritebatch;
Font 
	font;

const int TEXTURE_SIZE = 256; // Cubic texture
const int NUM_SLICES = 16; // Volume rendered as <num_slices> alphablended textured quads
const int LOCAL_SIZE = 4; // Shared for all axes
const int NUM_WORK_GROUPS = TEXTURE_SIZE / LOCAL_SIZE; // Same

GLuint createPyroclasticVolume()
{
	GLuint handle;
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_3D, handle);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0,
		GL_R32F,
		TEXTURE_SIZE, TEXTURE_SIZE, TEXTURE_SIZE, 0,
		GL_RED,
		GL_FLOAT,
		NULL);

	shader_genvolume.begin();
	shader_genvolume.setUniform("dimension", TEXTURE_SIZE);
	shader_genvolume.setUniform("center", vec3(TEXTURE_SIZE / 2.0f + 0.5f));
	shader_genvolume.setUniform("outTex", 0);
	glBindImageTexture(0, handle, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glDispatchCompute(NUM_WORK_GROUPS, NUM_WORK_GROUPS, NUM_WORK_GROUPS);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	return handle;
}

bool load()
{
	string paths[] = { "./demo/32volume/genvolume.cs" };
	GLenum types[] = { GL_COMPUTE_SHADER };
	if (!shader_simple.loadAndLinkFromFile("./demo/32volume/simple") ||
		!shader_planes.loadAndLinkFromFile("./demo/32volume/planes") ||
		!shader_genvolume.loadFromFile(paths, types, 1) ||
		!shader_slices.loadAndLinkFromFile("./demo/32volume/slices"))
		return false;

	if (!shader_genvolume.linkAndCheckStatus())
		return false;

	if (!font.loadFromFile("./data/fonts/proggytinyttsz_8x12.png"))
		return false;

	spritebatch.create();
	spritebatch.setFont(font);

	return true;
}

void free()
{
	glDeleteTextures(1, &tex_volume);
	buf_quads.dispose();
	buf_cube.dispose();
	shader_genvolume.dispose();
	shader_simple.dispose();
	shader_slices.dispose();
	shader_planes.dispose();
	font.dispose();
	spritebatch.dispose();
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{	
	vao.create();
	vao.bind();

	tex_volume = createPyroclasticVolume();
	Mesh quads;
	for (int i = 0; i < NUM_SLICES; ++i)
	{
		float z = -1.0 + 2.0 * i / NUM_SLICES;
		Mesh quad;
		quad.addPosition(-1.0f, -1.0f, z); quad.addTexel(0.0f, 0.0f);
		quad.addPosition( 1.0f, -1.0f, z); quad.addTexel(1.0f, 0.0f);
		quad.addPosition( 1.0f,  1.0f, z); quad.addTexel(1.0f, 1.0f);
		quad.addPosition(-1.0f,  1.0f, z); quad.addTexel(0.0f, 1.0f);
		uint32 indices[] = { 0, 1, 2, 2, 3, 0 };
		quad.addIndices(indices, 6);
		quads.addMesh(quad);
	}
	buf_quads.create(quads);
	buf_cube.create(Mesh::genUnitCube(false, false, true));

	mat_projection = glm::perspective(PI / 4.0f, ctx.getWidth() / (float)ctx.getHeight(), 0.1f, 5.0f);
}

void keyReleased(int mod, SDL_Keycode key)
{
	if (key == SDLK_r)
	{
		shader_genvolume.dispose();
		string paths[] = { "./demo/32volume/genvolume.cs" };
		GLenum types[] = { GL_COMPUTE_SHADER };
		if (!shader_genvolume.loadFromFile(paths, types, 1))
			exit(EXIT_FAILURE);

		if (!shader_genvolume.linkAndCheckStatus())
			exit(EXIT_FAILURE);

		glDeleteTextures(1, &tex_volume);
		tex_volume = createPyroclasticVolume();
	}
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	mat_view = translate(0.0f, 0.0f, -2.0f) * rotateX(-0.5f) * rotateY(sin(ctx.getElapsedTime() * 0.3f));
}

float time_render_begin = 0.0f;
void render(Renderer &gfx, Context &ctx, double dt)
{
	float time_now = ctx.getElapsedTime();
	float frametime = time_now - time_render_begin;
	time_render_begin = time_now;
	gfx.clear(0x2a2a2aff, 1.0);
	gfx.setCullState(CullStates::CullCounterClockwise);

	gfx.beginCustomShader(shader_simple);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("model", scale(1.0f));
	gfx.setRasterizerState(RasterizerStates::FillBoth);
	buf_cube.draw();

	gfx.beginCustomShader(shader_slices);
	gfx.setCullState(CullStates::CullNone);
	gfx.setRasterizerState(RasterizerStates::FillBoth);
	gfx.setBlendState(BlendStates::AlphaBlend);	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, tex_volume);
	gfx.setUniform("texVolume", 0);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("model", scale(0.5f));
	buf_quads.draw();
	gfx.endCustomShader();
	gfx.setBlendState(BlendStates::Default);

	spritebatch.begin();
	Text text;
	text << "frametime: " << int(frametime * 1000.0f) << "ms\n";
	text << "resolution: " << TEXTURE_SIZE << "\n";
	text << "num slices: " << NUM_SLICES;
	spritebatch.drawString(text.getString(), vec2(5.0, 5.0), Color::fromHex(0x787878ff), 1.0f);
	spritebatch.end();
}