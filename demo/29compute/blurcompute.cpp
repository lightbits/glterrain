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
	tex,
	tex_blurred;
VertexArray vao;
BufferObject vbo;

const int NUM_GROUPS_X = 16;
const int NUM_GROUPS_Y = 16;

bool load()
{
	GLenum types[] = { GL_COMPUTE_SHADER };
	string paths[] = { "./demo/29compute/blurcompute.cs" };
	if (!shader_display.loadFromFile("./demo/29compute/texturems") ||
		!shader_compute.loadFromFile(paths, types, 1))
		return false;

	if (!shader_display.linkAndCheckStatus() ||
		!shader_compute.linkAndCheckStatus())
		return false;

	if (!tex.loadFromFile("./data/textures/dungeon2.png", GL_RGBA16F))
		return false;

	tex.setTexParameteri(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	tex_blurred.create(0, GL_RGBA16F, tex.getWidth(), tex.getHeight(), GL_RGB, GL_FLOAT, NULL);
	tex_blurred.setTexParameteri(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

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
	glBindImageTexture(0, tex.getHandle(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
	glBindImageTexture(1, tex_blurred.getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
	glDispatchCompute(
		tex.getWidth() / NUM_GROUPS_X,
		tex.getHeight() / NUM_GROUPS_Y,
		1);

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
		glBindImageTexture(0, tex_blurred.getHandle(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
	else
		glBindImageTexture(0, tex.getHandle(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
}