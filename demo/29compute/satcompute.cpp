/*
This one blurs an image by averaging the colors of 9 neighbour pixels.
Here we make the local work group those 9 pixels, and thus have to use
256x256x1 work groups in total - one for each pixel.

The work groups will then work on the 3x3 grid around each pixel.

The compute shader works by summing the colors of the input texture.
We have a problem though!
The work items in a group are all executed in parallel!!

To add a sample to the texture we need to fetch the current sum.
It will not work if we just sample the output texture for each
work item, because the sample will be the same for all work items.

Resources
=========
http://programmers.stackexchange.com/questions/188944/opengl-image-load-store-atomics-applications
http://www.opengl.org/wiki/Compute_Shader
http://www.opengl.org/wiki/Image_Load_Store
https://www.opengl.org/discussion_boards/showthread.php/180692-OpenGL-Compute-Shaders-vs-OpenCL
*/

#include "app.h"

ShaderProgram 
	shader_display,
	shader_compute;
Texture2D 
	tex_in,
	tex_out;
VertexArray vao;
BufferObject vbo;

const int NUM_GROUPS_X = 16;
const int NUM_GROUPS_Y = 16;

bool load()
{
	GLenum types[] = { GL_COMPUTE_SHADER };
	string paths[] = { "./demo/29compute/satcompute.cs" };
	if (!shader_display.loadFromFile("./demo/29compute/texturems") ||
		!shader_compute.loadFromFile(paths, types, 1))
		return false;

	if (!shader_display.linkAndCheckStatus() ||
		!shader_compute.linkAndCheckStatus())
		return false;

	float data[] = {
		0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
	};

	tex_in.create(0, GL_RGBA16F, 4, 4, GL_RED, GL_FLOAT, data);
	tex_in.setTexParameteri(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	tex_out.create(0, GL_RGBA16F, tex_in.getWidth(), tex_in.getHeight(), GL_RGB, GL_FLOAT, NULL);
	tex_out.setTexParameteri(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	return true;
}

void free()
{
	vao.dispose();
	vbo.dispose();
}

void init(Renderer &gfx, Context &ctx)
{
	// Use the compute shader to work magic on the texture
	gfx.beginCustomShader(shader_compute);
	gfx.setUniform("inTex", 0);
	gfx.setUniform("outTex", 1);
	glBindImageTexture(0, tex_in.getHandle(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
	glBindImageTexture(1, tex_out.getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
	glDispatchCompute(4, 4, 1);

	// Normally we can't be sure what in order shaders are executed
	// For example, the texture display shader below could run before this one!
	// So we need to make sure that this shader has finished running before
	// using the output image in the next shader.
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	tex_out.bind();
	vec4 pixels[16];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels);
	for (int i = 0; i < 16; ++i)
	{
		std::cout << pixels[i].r << " ";
		if ((i + 1) % 4 == 0)
			std::cout << '\n';
	}
	std::cout << '\n';

	const float vertices[] = {	
		-1.0f, -1.0f,
		+1.0f, -1.0f,
		+1.0f, +1.0f,
		+1.0f, +1.0f,
		-1.0f, +1.0f,
		-1.0f, -1.0f
	};

	vbo.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	vbo.bind();
	vbo.bufferData(sizeof(vertices), vertices);

	vao.create();
	vao.bind();
}

void update(Renderer &gfx, Context &ctx, double dt)
{

}

void render(Renderer &gfx, Context &ctx, double dt)
{
	// Render the texture
	gfx.beginCustomShader(shader_display);
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setCullState(CullStates::CullNone);
	gfx.clear(0x2a2a2aff, 1.0);
	gfx.setUniform("tex", 0);
	gfx.setAttributefv("position", 2, 2, 0);

	if (ctx.isKeyPressed('a'))
		glBindImageTexture(0, tex_out.getHandle(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
	else
		glBindImageTexture(0, tex_in.getHandle(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
}