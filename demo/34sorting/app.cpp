#include "app.h"
#include <common/noise.h>
using namespace transform;

ShaderProgram shader_sort, shader_sprite;
VertexArray vao;
BufferObject 
	buffer_quad,
	buffer_pos;
const int LOCAL_SIZE = 1;
const int NUM_SPRITES = 8;
const int NUM_GROUPS = NUM_SPRITES / LOCAL_SIZE;

bool loadComputeShader(ShaderProgram &shader, const string &computePath)
{
	string paths[] = { computePath };
	GLenum types[] = { GL_COMPUTE_SHADER };
	return shader.loadFromFile(paths, types, 1);
}

bool load()
{
	if (!loadComputeShader(shader_sort, "./demo/34sorting/oddevenmerge.cs") ||
		!shader_sprite.loadFromFile("./demo/34sorting/sprite"))
		return false;

	if (!shader_sort.linkAndCheckStatus() ||
		!shader_sprite.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{
	vao.dispose();
}

/*
Generate a row of particles with random z-depth, ranging from -1 to +1.
*/
void initParticles(Renderer &gfx, Context &ctx)
{
	buffer_pos.bind();
	vec4 *position = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 
		NUM_SPRITES * sizeof(vec4), 
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	std::cout << "random: ";
	for (int i = 0; i < NUM_SPRITES; ++i)
	{
		float z = -1.0f + 2.0f * frand();
		position[i] = vec4(0.0, 0.0, z, 1.0);
		std::cout << z << " ";
	}
	std::cout << std::endl;
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

/*
The sorting shader takes the position buffer as input, and sorts the buffer
in order of increasing z-value. The z-value is converted, in shader, to an integer by
key = int((z - zMin) / (zMax - zMin)).
*/
void sort(Renderer &gfx, Context &ctx)
{
	gfx.beginCustomShader(shader_sort);
	for (int i = 0; i < NUM_SPRITES; ++i)
	{
		gfx.setUniform("offset", i % 2);
		gfx.setUniform("zMin", -1.0f);
		gfx.setUniform("zMax", 1.0f);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer_pos.getHandle());
		glDispatchCompute(NUM_SPRITES / 2, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
}

void computestep(int *data_in, int *data_out, int i, int pstage, int ppass)
{
	int j = i % (pstage * 2);
	int compare;
	if (j < ppass % pstage || j > 2 * pstage - (ppass % pstage) - 1)
	{
		compare = 0;
	}
	else
	{
		if (((j + (ppass % pstage) / ppass) % 2) < 1)
			compare = 1;
		else
			compare = -1;
	}

	int otherIndex = i + compare * ppass;
	int otherKey = data_in[otherIndex];
	int thisKey = data_in[i];

	data_out[i] = thisKey * compare < otherKey * compare ? thisKey : otherKey;
}

void mergesort()
{
	const int NUM_ELEMENTS = 8;
	int data[NUM_ELEMENTS] = { 0, 5, 9, 8, 2, 4, 1, 6 };
	int sorted[NUM_ELEMENTS];
	int *data_ping = data;
	int *data_pong = sorted;
	int stage = 0;
	int pass = 0;
	while (stage < 3)
	{
		pass--;
		if (pass < 0)
		{
			stage++;
			pass = stage;
		}

		for (int i = 0; i < NUM_ELEMENTS; ++i)
		{
			computestep(data_ping, data_pong, i, 1 << stage, 1 << pass);
		}

		int *temp = data_ping;
		data_ping = data_pong;
		data_pong = temp;
	}



	std::cout << "sorted: ";
	for (int i = 0; i < NUM_ELEMENTS; ++i)
		std::cout << data_pong[i] << " ";
	std::cout << '\n';
}

void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();

	mergesort();

	buffer_pos.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_SPRITES * sizeof(vec4), NULL);

	//initParticles(gfx, ctx);
	//sort(gfx, ctx);

	float quad[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f
	};
	buffer_quad.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(quad), quad);
}

bool sorted = false;
void update(Renderer &gfx, Context &ctx, double dt)
{
	if (ctx.isKeyPressed('s') && !sorted)
	{
		sorted = true;
		sort(gfx, ctx);

		// Read back the values to console (for convenience)
		buffer_pos.bind();
		vec4 *result = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_SPRITES * sizeof(vec4), GL_MAP_READ_BIT);
		std::cout << "\nsorted: ";
		for (int i = 0; i < NUM_SPRITES; ++i)
			std::cout << result[i].z << " ";
		std::cout << std::endl;
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.clear(0x2a2a2aff, 1.0);
	gfx.beginCustomShader(shader_sprite);
	//gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setBlendState(BlendStates::AlphaBlend);
	gfx.setUniform("projection", glm::perspective(PI / 2.0f, 720.0f / 480.0f, 0.1f, 5.0f));
	gfx.setUniform("view", translate(0.0f, 0.0f, -3.0f) * rotateX(-0.45f) * rotateY(sin(ctx.getElapsedTime())));
	gfx.setUniform("model", mat4(1.0f));
	buffer_quad.bind(GL_ARRAY_BUFFER);
	gfx.setAttributefv("position", 3, 0, 0);
	buffer_pos.bind(GL_ARRAY_BUFFER);
	gfx.setAttributefv("spritePosition", 4, 0, 0);
	gfx.setAttributeDivisor("spritePosition", 1);
	glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, NUM_SPRITES, 0);
}