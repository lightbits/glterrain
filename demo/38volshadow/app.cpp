#include "app.h"

ShaderProgram 
	shader_prefix,
	shader_fill,
	shader_sampler;

VertexArray vao;
GLuint tex_input, tex_output;
BufferObject position_buffer;

bool loadComputeShader(ShaderProgram &shader, const string &computePath)
{
	string paths[] = { computePath };
	GLenum types[] = { GL_COMPUTE_SHADER };
	return shader.loadFromFile(paths, types, 1);
}

bool load()
{
	if (!loadComputeShader(shader_prefix, "./demo/38volshadow/prefix.cs") ||
		!loadComputeShader(shader_fill, "./demo/38volshadow/filltex.cs") ||
		!shader_sampler.loadFromFile("./demo/38volshadow/sampler"))
		return false;

	if (!shader_prefix.linkAndCheckStatus() ||
		!shader_fill.linkAndCheckStatus() ||
		!shader_sampler.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{

}

GLuint gen_texture(int w, int h, int d)
{
	GLuint id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_3D, id);
	glTexStorage3D(GL_TEXTURE_3D, 1, GL_R8, w, h, d);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_3D, 0);
	return id;
}

void fill(Renderer &gfx, Context &ctx, int mask, GLuint tex, int w, int h, int d)
{
	gfx.beginCustomShader(shader_fill);
	gfx.setUniform("mask", mask);
	glBindImageTexture(0, tex, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8);
	glDispatchCompute(w, h, d);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

const uint32 num_particles = 1 << 10;
void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();

	const uint32 width = 512;
	const uint32 height = 512;
	const uint32 depth = 32;
	tex_input = gen_texture(width, height, depth);
	tex_output = gen_texture(width, height, depth);

	fill(gfx, ctx, 1, tex_input, width, height, depth);
	fill(gfx, ctx, 0, tex_output, width, height, depth);

	vec2 *data = new vec2[num_particles];
	for (int i = 0; i < num_particles; ++i)
		data[i] = vec2((rand() % 1024) / 1024.0, (rand() % 1024) / 1024.0);
	position_buffer.create(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, num_particles * sizeof(vec2), data);
	position_buffer.bind();
}

void update(Renderer &gfx, Context &ctx, double dt)
{


}

void render(Renderer &gfx, Context &ctx, double dt)
{
	GLuint64 start, stop;
	GLuint queryID[2];
	glGenQueries(2, queryID);
	glQueryCounter(queryID[0], GL_TIMESTAMP);

	glBindTexture(GL_TEXTURE_3D, tex_input);
	gfx.beginCustomShader(shader_sampler);
	gfx.setUniform("tex", 0);
	gfx.setAttributefv("position", 2, 0, 0);

	for (int i = 0; i < 32; ++i)
	{
		gfx.setUniform("slice", i);
		glDrawArrays(GL_POINTS, i * num_particles / 32, num_particles / 32);
	}

	glQueryCounter(queryID[1], GL_TIMESTAMP);
	GLint timeAvail = 0;
	while (!timeAvail) glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &timeAvail);
	glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &start);
	glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &stop);
	printf("Time spent on the GPU: %.2f ms\t\t\r", (stop - start) / 1000000.0);
}