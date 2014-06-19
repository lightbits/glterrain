#include "app.h"
#include <common/noise.h>
using namespace transform;

VertexArray vao;
ShaderProgram shader_intervals, shader_raycast;
Model cube;
MeshBuffer cube_buffer, ssquad_buffer;
mat4 mat_projection, mat_view;
RenderTexture tex_front, tex_back;
GLuint tex_volume;

GLuint CreatePyroclasticVolume(int n, float r)
{
	GLuint handle;
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_3D, handle);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	unsigned char *data = new unsigned char[n*n*n];
	unsigned char *ptr = data;

	float frequency = 3.0f / n;
	float center = n / 2.0f + 0.5f;

	for (int x = 0; x < n; ++x) {
		for (int y = 0; y < n; ++y) {
			for (int z = 0; z < n; ++z) {
				float dx = center - x;
				float dy = center - y;
				float dz = center - z;

				float d = sqrtf(dx*dx + dy*dy + dz*dz) / (n);
				float off = fabsf(cos(dz * 0.25f) * cos(dy * 0.15f) * sin(dx * 0.1f)) * d * 0.25f;
				bool isFilled = (d - off) < r;
				 *ptr++ = isFilled ? 255 : 0;
			}
		}
	}

	glTexImage3D(GL_TEXTURE_3D, 0,
		GL_R32F,
		n, n, n, 0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		data);

	delete[] data;
	return handle;
}

bool load()
{
	if (!shader_intervals.loadAndLinkFromFile("./demo/32volume/intervals") ||
		!shader_raycast.loadAndLinkFromFile("./demo/32volume/raycast"))
		return false;

	return true;
}

void free()
{
	cube_buffer.dispose();
	tex_front.dispose();
	tex_back.dispose();
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{	
	vao.create();
	vao.bind();

	cube_buffer.create(Mesh::genUnitCube(false, false, true));
	ssquad_buffer.create(Mesh::genScreenSpaceTexQuad());
	cube = Model(cube_buffer);

	tex_front.create(ctx.getWidth(), ctx.getHeight());
	tex_back.create(ctx.getWidth(), ctx.getHeight());
	tex_volume = CreatePyroclasticVolume(128, 0.2f);
	mat_projection = glm::perspective(PI / 4.0f, ctx.getWidth() / (float)ctx.getHeight(), 0.1f, 10.0f);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	mat_view = translate(0.0f, 0.0f, -2.0f) * rotateX(-0.5f) * rotateY(ctx.getElapsedTime() * 0.1f);
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.beginCustomShader(shader_intervals);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("view", mat_view);

	tex_front.begin();
	gfx.setCullState(CullStates::CullCounterClockwiseBack);
	gfx.clear(0x00000000, 1.0);
	cube.draw();
	tex_front.end();

	tex_back.begin();
	gfx.setCullState(CullStates::CullCounterClockwiseFront);
	gfx.clear(0x00000000, 1.0);
	cube.draw();
	tex_back.end();
	gfx.endCustomShader();

	gfx.beginCustomShader(shader_raycast);
	gfx.clear(0x2a2a2aff, 1.0);
	gfx.setBlendState(BlendStates::Additive);
	gfx.setCullState(CullStates::CullCounterClockwiseBack);
	tex_front.bindTexture(GL_TEXTURE0);
	gfx.setUniform("texFront", 0);
	tex_back.bindTexture(GL_TEXTURE1);
	gfx.setUniform("texBack", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, tex_volume);
	gfx.setUniform("texVolume", 2);
	ssquad_buffer.draw();
	gfx.endCustomShader();
	gfx.setBlendState(BlendStates::Default);
}