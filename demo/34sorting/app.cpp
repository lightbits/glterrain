#include "app.h"
using namespace transform;

ShaderProgram shader_sort, shader_sprite;
VertexArray vao;
BufferObject buffer_quad, buffer_pos;
const int LOCAL_SIZE = 1;
const int NUM_SPRITES = 4;
const int NUM_GROUPS = NUM_SPRITES / LOCAL_SIZE;

bool load()
{
	GLenum types[] = { GL_COMPUTE_SHADER };
	string paths[] = { "./demo/34sorting/oddeven.cs" };
	if (!shader_sort.loadFromFile(paths, types, 1) ||
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

//vector< vector<float> > split(vector<float> in, int pos)
//{
//	vector< vector<float> > buckets(2);
//	for (int i = 0; i < in.size(); ++i)
//	{
//		float n = in[i];
//		uint32 key = *(uint32*)&n;
//		if ((key >> pos) & 0x1)
//			buckets[1].push_back(n);
//		else
//			buckets[0].push_back(n);
//	}
//	return buckets;
//}
//
//vector<float> merge(vector< vector<float> > in)
//{
//	vector<float> result;
//	for (int i = 0; i < in.size(); ++i)
//	for (int j = 0; j < in[i].size(); ++j)
//		result.push_back(in[i][j]);
//	return result;
//}

void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();

	//vector<float> data;
	//data.push_back(170);
	//data.push_back(75.5);
	//data.push_back(75.55);
	//data.push_back(90);
	//data.push_back(2.05123981);
	//data.push_back(2.012341);
	//data.push_back(24);
	//data.push_back(66);
	//for (int i = 0; i < 32; ++i)
	//{
	//	data = merge(split(data, i));
	//}
	//std::cout << "sorted: ";
	//for (int i = 0; i < BUFFER_LEN; ++i)
	//	std::cout << data[i] << " ";

	buffer_pos.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, NUM_SPRITES * sizeof(vec4), NULL);
	buffer_pos.bind();
	vec4 *position = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_SPRITES * sizeof(vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int i = 0; i < NUM_SPRITES; ++i)
		position[i] = vec4(0.0, 0.0, -1.0 + 2.0 * i / NUM_SPRITES, 1.0);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	// Create a buffer filled with random values
	//int data[BUFFER_LEN];
	//int sorted[BUFFER_LEN];
	//std::cout << "random: ";
	//for (int i = 0; i < BUFFER_LEN; ++i)
	//{
	//	data[i] = rand() % BUFFER_LEN;
	//	sorted[i] = data[i];
	//	std::cout << data[i] << " ";
	//}
	//std::cout << std::endl;

	//BufferObject buffer_ping, buffer_pong;
	//buffer_ping.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, BUFFER_LEN * sizeof(int), data);

	//// Sort by key
	//gfx.beginCustomShader(shader_sort);
	//for (int i = 0; i < BUFFER_LEN; ++i)
	//{
	//	gfx.setUniform("offset", i % 2);
	//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer_ping.getHandle());
	//	glDispatchCompute(8, 1, 1);
	//	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	//}

	//// Read back the values
	//int *result = (int*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, BUFFER_LEN * sizeof(int), GL_MAP_READ_BIT);
	//std::cout << "\nsorted: ";
	//for (int i = 0; i < BUFFER_LEN; ++i)
	//	std::cout << result[i] << " ";
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

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

void update(Renderer &gfx, Context &ctx, double dt)
{

}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.clear(0x2a2a2aff, 1.0);
	gfx.beginCustomShader(shader_sprite);
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
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
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
}