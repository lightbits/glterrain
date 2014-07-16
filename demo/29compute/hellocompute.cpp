/*
This demo uses compute shaders somehow, an OpenGL 4.3 feature!

Resources:
http://wili.cc/blog/opengl-cs.html
http://mcnopper.blogspot.no/
http://media.siggraph.org/education/conference/S2012_Materials/ComputeShader_6pp.pdf
http://www.opengl.org/wiki/Compute_Shader
http://www.opengl.org/wiki/Image_Load_Store
http://onlygraphix.com/category/opengl/
http://github.prideout.net/modern-opengl-prezo/
http://cs.brown.edu/courses/cs195v/lecture/week6.pdf
http://books.google.no/books?id=odgdAAAAQBAJ&pg=PA444&lpg=PA444&dq=image2d+opengl+binding&source=bl&ots=waKrHZjXL6&sig=ps0mFYVJhp7yt1aUR2pFIj6T6hk&hl=no&sa=X&ei=vaKdU_avH7K0sQTbtoAI&ved=0CC8Q6AEwAjgK#v=onepage&q=image2d%20opengl%20binding&f=false
http://www.g-truc.net/doc/OpenGL%204.3%20review.pdf

Cool stuff
https://www.youtube.com/watch?v=ycSOnGzbNO4
https://www.youtube.com/watch?v=_K2Wx7lW3fY
https://github.com/Themaister/boxes

####################
#### OpenGL 4.3 ####
####################

### Textures ###
Textures might not work until setTexParameter has been called on them
Internalformats are new, including
	GL_RGBA, GL_RGB, GL_RGB32UI, GL_RGBA32F, GL_RGBA16F, ...

### Compute ###
The compute shader space is sort of like a grid (1D, 2D or 3D) containing blocks 
(1D, 2D or 3D) called (local) work groups.

Each local work group consist of a grid of work items.
The size of the local work groups is set by the local_size_x/y/z layout.

Work groups can be launched through glDispatchCompute(num_groups_x/y/z).
The compute shader is then run once for item in each group.

In the shader we can then find out where we are using these
gl_WorkGroupID			- the 3D index of a work group
gl_LocalInvocationID	- the 3D index of the work item with a group
gl_GlobalInvocationID	- the 3D index of the work item relative to the "global grid"

See hellocompute.cs for further explanation of the indices.
*/

#include "app.h"

ShaderProgram 
	shader_display,
	shader_compute;
Texture2D tex, tex_noise;
VertexArray vao;
BufferObject vbo;

const int TEXTURE_SIZE_X = 512;
const int TEXTURE_SIZE_Y = 512;
const int NUM_GROUPS_X = 16;
const int NUM_GROUPS_Y = 16;

bool load()
{
	GLenum types[] = { GL_COMPUTE_SHADER };
	string paths[] = { "./demo/29compute/hellocompute.cs" };
	if (!shader_display.loadFromFile("./demo/29compute/texture") ||
		!shader_compute.loadFromFile(paths, types, 1))
		return false;

	if (!shader_display.linkAndCheckStatus() ||
		!shader_compute.linkAndCheckStatus())
		return false;

	if (!tex_noise.loadFromFile("./data/textures/rgbanoise.png"))
		return false;

	return true;
}

void free()
{
	vao.dispose();
	vbo.dispose();
}

void init(Renderer &gfx, Context &ctx)
{
	// The compute shader will draw into this empty texture
	tex.create(0, GL_RGBA16F, TEXTURE_SIZE_X, TEXTURE_SIZE_Y, GL_RGB, GL_FLOAT, NULL);
	tex.setTexParameteri(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

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

	// Use the compute shader to work magic on the texture
	gfx.beginCustomShader(shader_compute);
	gfx.setUniform("tex", 0);
	gfx.setUniform("texNoise", 1);
	glBindImageTexture(0, tex.getHandle(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glBindImageTexture(1, tex_noise.getHandle(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
	glDispatchCompute(
		TEXTURE_SIZE_X / NUM_GROUPS_X,
		TEXTURE_SIZE_Y / NUM_GROUPS_Y,
		1);

	// Render the texture
	gfx.beginCustomShader(shader_display);
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setCullState(CullStates::CullNone);
	gfx.clear(0x2a2a2aff, 1.0);
	gfx.setUniform("tex", 0);
	gfx.setAttributefv("position", 2, 2, 0);
	glBindImageTexture(0, tex.getHandle(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);

	//ctx.screenshot("compute03.png");
}

void update(Renderer &gfx, Context &ctx, double dt)
{

}

void render(Renderer &gfx, Context &ctx, double dt)
{
	
}