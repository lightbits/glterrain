/*
This example uses deferred shading to render a scene with loads of lights.
Well, actually it's only 2-3 lights limited to each fragment. But the limiting
is done by doing the lighting computations in a second pass.

First pass: 
	Render the geometry, and store the information needed to do lighting
	in the G-buffer (geometry buffer). Here I store the vertex position and normal
	in view-space, into two seperate textures.

Second pass:
	Render each pointlight as a quad, with appropriate spacing between
	each light. The lighting is done by sampling the previous two textures,
	and performing the usual phong shading.

	By rendering each pointlight as a quad (scaled and projected to the screen),
	the number of fragments calculated can be decreased, as opposed to rendering
	a fullscreen quad for each light.

	The blendmode is set to additive, so that each light adds its contribution.
*/

#include "app.h"

MeshBuffer 
	cube_buffer,
	sphere_buffer;

Model 
	cube,
	sphere;

ShaderProgram
	shader_fp,
	shader_sp;

mat4 
	projection,
	view;

VertexArray vao;

// Framebuffer to contain the output textures
Framebuffer framebuffer;

// Renderbuffer to enable depth-sorting for the fb
Renderbuffer renderbuffer;

// Textures to hold position and normal data
Texture 
	tex_p, // Position
	tex_n, // Normal
	tex_d; // Diffuse

const int num_lights = 48;
float light_r[num_lights]; // Radii
mat4 light_m[num_lights]; // World-transformations
vec3 light_p[num_lights]; // Positions
vec3 light_d[num_lights]; // Diffuse colors
vec3 light_s[num_lights]; // Specular colors

bool load()
{
	if (!shader_fp.loadFromFile("./demo/21deferred/first_pass") ||
		!shader_sp.loadFromFile("./demo/21deferred/second_pass"))
		return false;

	// setup output variables
	shader_fp.bindFragDataLocation("out_p", 0);
	shader_fp.bindFragDataLocation("out_n", 1);
	shader_fp.bindFragDataLocation("out_d", 2);

	if (!shader_fp.linkAndCheckStatus() ||
		!shader_sp.linkAndCheckStatus())
		return false;
	return true;
}

void free()
{

}

void init(Renderer &gfx, Context &ctx)
{
	Mesh mesh = Mesh::genUnitCube(false, true);
	cube_buffer = MeshBuffer(mesh);
	cube = Model(cube_buffer);

	sphere_buffer = MeshBuffer(Mesh::genUnitSphere(8, 8));
	sphere = Model(sphere_buffer);

	// Default vertex array object
	vao.create();
	vao.bind();

	projection = glm::perspective(45.0f, (float)ctx.getWidth() / ctx.getHeight(), 0.1f, 20.0f);

	// Create texture that will hold fragment position output
	tex_p.create2d(0, GL_RGB16F, ctx.getWidth(), ctx.getHeight(), GL_RGB, GL_UNSIGNED_BYTE, NULL);
	tex_p.bind();
	tex_p.setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	tex_p.unbind();

	// Create texture that will hold fragment normal output
	tex_n.create2d(0, GL_RGB16F, ctx.getWidth(), ctx.getHeight(), GL_RGB, GL_UNSIGNED_BYTE, NULL);
	tex_n.bind();
	tex_n.setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	tex_n.unbind();

	// Create texture that will hold fragment diffuse output
	tex_d.create2d(0, GL_RGB16F, ctx.getWidth(), ctx.getHeight(), GL_RGB, GL_UNSIGNED_BYTE, NULL);
	tex_d.bind();
	tex_d.setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	tex_d.unbind();

	// The renderbuffer is actually just used to depth testing
	renderbuffer.create();
	renderbuffer.bind();
	renderbuffer.storage(GL_DEPTH_COMPONENT, ctx.getWidth(), ctx.getHeight());
	renderbuffer.unbind();

	// Create framebuffer that will be used to render to these textures
	framebuffer.create();
	framebuffer.bind();
	framebuffer.attachTexture2D(GL_COLOR_ATTACHMENT0, tex_p, 0); // position is colornumber 0
	framebuffer.attachTexture2D(GL_COLOR_ATTACHMENT1, tex_n, 0); // normal is colornumber 1
	framebuffer.attachTexture2D(GL_COLOR_ATTACHMENT2, tex_d, 0); // normal is colornumber 1
	framebuffer.attachRenderbuffer(GL_DEPTH_ATTACHMENT, renderbuffer);

	// Specify output targets
	GLenum drawBuffers[] = { 
		GL_COLOR_ATTACHMENT0, 
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2 
	};
	glDrawBuffers(3, drawBuffers);

	GLenum status = framebuffer.checkStatus();
	if (status != GL_FRAMEBUFFER_COMPLETE)
		throw std::runtime_error("Framebuffer not complete");

	//renderbuffer.unbind();
	framebuffer.unbind();

	// Setup light matrices and world positions
	for (int i = 0; i < num_lights; ++i)
	{
		float x = 3.0f * sin(4.0f * TWO_PI * i / num_lights);
		float y = 0.3f + 0.1f * cos(4.0f * TWO_PI * i / num_lights);
		float z = -4.0f + 9.0f * i / num_lights;
		light_p[i] = vec3(x, y, z);
		light_d[i] = vec3(
			0.5 + 0.5 * sin(TWO_PI * i / num_lights),
			0.5 + 0.5 * sin(PI / 2.0f + TWO_PI * i / num_lights),
			0.5 + 0.5 * sin(PI + TWO_PI * i / num_lights));
		light_s[i] = vec3(1.0, 1.0, 1.0);
		light_r[i] = 0.6f + 0.1f * sin(i);
		light_m[i] = transform::translate(light_p[i]) * transform::scale(light_r[i]);

	}

	// The coolest light of the bunch
	light_r[3] = 8.0f;
	light_p[3] = vec3(0.0f, 3.0f, 0.0f);
	light_d[3] = vec3(0.8f, 0.7f, 0.5f);
	light_m[3] = transform::translate(light_p[3]) * transform::scale(light_r[3]);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	view = 
		transform::translate(0.0f, -1.5f, -10.0f) * 
		transform::rotateX(-0.44f) * 
		transform::rotateY(sin(ctx.getElapsedTime() * 0.5f) * 0.8f);
}

void renderGeometry(Renderer &gfx, Context &ctx, double dt)
{
	gfx.setUniform("diffuse", vec3(1.0));
	cube.transform.push();
	cube.transform.translate(0.0f, 0.4f, 0.0f);
	cube.transform.rotateX(sin(ctx.getElapsedTime() * 1.5f));
	cube.transform.rotateY(ctx.getElapsedTime());
	cube.transform.scale(0.7f);
	cube.draw();
	cube.transform.pop();

	// Floor
	gfx.setUniform("diffuse", vec3(0.76f, 0.75f, 0.5f));
	cube.transform.push();
	cube.transform.scale(8.0f, 0.2f, 8.0f);
	cube.draw();
	cube.transform.pop();

	// Back wall
	gfx.setUniform("diffuse", vec3(0.76f, 0.75f, 0.5f));
	cube.transform.push();
	cube.transform.translate(0.0f, 1.0f, -4.0f);
	cube.transform.scale(8.0f, 2.0f, 0.2f);
	cube.draw();
	cube.transform.pop();

	// Left wall
	gfx.setUniform("diffuse", vec3(0.15f, 0.48f, 0.09f));
	cube.transform.push();
	cube.transform.translate(-4.0f, 1.0f, 0.0f);
	cube.transform.scale(0.2f, 2.0f, 8.0f);
	cube.draw();
	cube.transform.pop();

	// Right wall
	gfx.setUniform("diffuse", vec3(0.63f, 0.06f, 0.04f));
	cube.transform.push();
	cube.transform.translate(+4.0f, 1.0f, 0.0f);
	cube.transform.scale(0.2f, 2.0f, 8.0f);
	cube.draw();
	cube.transform.pop();

	for (int i = 0; i < num_lights; ++i)
	{
		gfx.setUniform("diffuse", light_d[i] * 10.0f);
		cube.transform.push();
		cube.transform.multiply(light_m[i]);
		cube.transform.scale(0.2f);
		cube.draw();
		cube.transform.pop();
	}
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	// Geometry pass
	gfx.beginCustomShader(shader_fp);
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setBlendState(BlendStates::Default);
	gfx.setClearDepth(1.0);
	gfx.setClearColor(0.0f, 0.0f, 0.0f);
	framebuffer.bind();
	gfx.clearColorAndDepth();
	gfx.setUniform("projection", projection);
	gfx.setUniform("view", view);
	renderGeometry(gfx, ctx, dt);
	gfx.endCustomShader();
	framebuffer.unbind();

	// Deferred pass
	gfx.setDepthTestState(DepthTestStates::Disabled);
	gfx.setCullState(CullStates::CullNone);
	gfx.setBlendState(BlendStates::Additive);
	gfx.setClearColor(0.1f, 0.1f, 0.1f);
	gfx.clearColorBuffer();

	gfx.beginCustomShader(shader_sp);
	glActiveTexture(GL_TEXTURE0); tex_p.bind();
	glActiveTexture(GL_TEXTURE1); tex_n.bind();
	glActiveTexture(GL_TEXTURE2); tex_d.bind();
	gfx.setUniform("projection", projection);
	gfx.setUniform("view", view);
	gfx.setUniform("tex_p", 0);
	gfx.setUniform("tex_n", 1);
	gfx.setUniform("tex_d", 2);

	// Draw each point light as a sphere
	// Anything within a sphere will be lit
	for (int i = 0; i < num_lights; ++i)
	{
		// Yeah this is pretty bad for parallelism!
		gfx.setUniform("light_p", light_p[i]);
		gfx.setUniform("light_d", light_d[i]);
		gfx.setUniform("light_r", light_r[i]);
		gfx.setUniform("light_s", light_s[i]);
		gfx.setUniform("model", light_m[i] * transform::rotateY(-sin(ctx.getElapsedTime() * 0.5f) * 0.8f) * 
			transform::rotateX(+0.34f)); // For transforming the quad to cover the light range

		gfx.drawQuad(-1.0f, -1.0f, 2.0f, 2.0f);
	}

	gfx.endCustomShader();
}