#include "app.h"
#include "sort.h"

BufferObject buf_input, buf_keys, buf_sorted;

bool load()
{
	if (!sort_init())
		return false;
	return true;
}

void free()
{

}

void init(Renderer &gfx, Context &ctx)
{
	float *input = new float[num_keys];
	for (int i = 0; i < num_keys; ++i)
		input[i] = 10.0 * (rand() % 65536) / 65535.0;

	buf_input.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(GLfloat), input);
	buf_keys.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(GLuint), NULL);
	buf_sorted.create(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW, num_keys * sizeof(GLuint), NULL);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	gen_keys(buf_input.getHandle(), buf_keys.getHandle());
	radix_sort(buf_keys.getHandle(), buf_sorted.getHandle());
}

void render(Renderer &gfx, Context &ctx, double dt)
{

}