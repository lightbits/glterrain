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
#include <gl/gbuffer.h>

MeshBuffer cube_buffer;
Model cube;

ShaderProgram
	shader_fp,
	shader_sp;

mat4 
	projection,
	view;

VertexArray vao;
GBuffer gbuffer;

const int num_lights = 32;
float light_r[num_lights]; // Radii
mat4 light_m[num_lights]; // World-transformations
vec3 light_p[num_lights]; // Positions
vec3 light_d[num_lights]; // Diffuse colors
vec3 light_s[num_lights]; // Specular colors
vec3 light_v[num_lights]; // Velocities

bool load()
{
	if (!shader_fp.loadFromFile("./demo/21deferred/first_pass") ||
		!shader_sp.loadFromFile("./demo/21deferred/second_pass"))
		return false;

	// Output variables (color numbers)
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

	gbuffer.create(ctx.getWidth(), ctx.getHeight());

	// Default vertex array object
	vao.create();
	vao.bind();

	projection = glm::perspective(45.0f, (float)ctx.getWidth() / ctx.getHeight(), 0.1f, 20.0f);

	// Setup light matrices and world positions
	for (int i = 0; i < num_lights; ++i)
	{
		// Grid positions
		float x = -2.5f + 2.0f * (i % 4);
		float y = 0.5f + 0.2f * cos(4.0f * TWO_PI * i / num_lights);
		float z = -3.6f + (i / 4);
		light_p[i] = vec3(x, y, z);

		// Nice colors
		light_d[i] = vec3(
			0.5 + 0.5 * sin(TWO_PI * i / num_lights),
			0.5 + 0.5 * sin(PI / 2.0f + TWO_PI * i / num_lights),
			0.5 + 0.5 * sin(PI + TWO_PI * i / num_lights));
		light_s[i] = vec3(1.0, 1.0, 1.0);
		light_r[i] = 0.6f + 0.1f * sin(float(i));
		light_m[i] = transform::translate(light_p[i]) * transform::scale(light_r[i]);
		light_v[i] = vec3(0.0, 0.8 * sin(float(i)), 0.0);

	}

	// The coolest light of the bunch
	light_r[3] = 12.0f;
	light_p[3] = vec3(0.0f, 4.0f, 0.0f);
	light_d[3] = vec3(0.8f, 0.7f, 0.5f);
	light_m[3] = transform::translate(light_p[3]) * transform::scale(light_r[3]);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	// Simulate bouncing of the lights
	//for (int i = 0; i < num_lights; ++i)
	//{
	//	light_p[i].y += light_v[i].y * dt;
	//	light_v[i] -= 2.0 * dt;
	//	if (light_p[i].y - 0.4 < 0.0)
	//	{
	//		light_p[i].y = 0.4;
	//		light_v[i] *= -0.99;
	//	}
	//	light_m[i] = transform::translate(light_p[i]) * transform::scale(light_r[i]);
	//}

	view = 
		transform::translate(0.0f, -1.5f, -10.0f) * 
		transform::rotateX(-0.44f) * 
		transform::rotateY(sin(ctx.getElapsedTime() * 0.5f) * 0.8f);
}

void renderGeometry(Renderer &gfx, Context &ctx, double dt)
{
	// Spinning cube
	gfx.setUniform("emissivity", 0.0f);
	gfx.setUniform("diffuse", vec3(0.63f, 0.06f, 0.04f));
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

	// Shining lights
	for (int i = 0; i < num_lights; ++i)
	{
		gfx.setUniform("diffuse", light_d[i]);
		gfx.setUniform("emissivity", 1.0f);
		cube.transform.push();
		cube.transform.translate(light_p[i]);
		cube.transform.scale(0.2f);
		cube.draw();
		cube.transform.pop();
	}
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	// Geometry pass
	gbuffer.begin();
	gfx.beginCustomShader(shader_fp);
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setBlendState(BlendStates::Default);
	gfx.setClearDepth(1.0);

	// Clear fragment output buffers to all zero (!)
	gfx.setClearColor(0.0f, 0.0f, 0.0f);
	gfx.clearColorAndDepth();
	gfx.setUniform("projection", projection);
	gfx.setUniform("view", view);
	renderGeometry(gfx, ctx, dt);
	gfx.endCustomShader();
	gbuffer.end();

	// Deferred pass
	gfx.setDepthTestState(DepthTestStates::Disabled);
	gfx.setCullState(CullStates::CullNone);
	gfx.setBlendState(BlendStates::Additive);
	gfx.setClearColor(0.1f, 0.1f, 0.1f);
	gfx.clearColorBuffer();

	gfx.beginCustomShader(shader_sp);
	gbuffer.bindTextures();
	gfx.setUniform("projection", projection);
	gfx.setUniform("view", view);
	gfx.setUniform("tex_p", 0);
	gfx.setUniform("tex_n", 1);
	gfx.setUniform("tex_d", 2);

	/*
	The light model matrix is used to transform the quad.
	Here I multiply by the inverse view rotation, to orient the quad towards the viewer.
	The last column of the view matrix is a translation vector, while the first 
	three define rotations and scaling. 

	Because we don't do any scaling here, we can get the rotation matrix with the 
	first three columns. A rotation matrix has orthonormal columns, which means that 
	its transpose is its inverse!
	*/
	mat4 inverse_view_r = mat4(view[0], view[1], view[2], vec4(0, 0, 0, 1));
	inverse_view_r = glm::transpose(inverse_view_r);

	// For each pointlight a quad is rendered that covers a bit more
	// than the range of the light. Anything inside the quad will be lit.
	BufferObject vbo;
	vbo.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	vbo.bind();
	float data[] = {
		-1.0f, -1.0f,
		+1.0f, -1.0f,
		+1.0f, +1.0f,
		+1.0f, +1.0f,
		-1.0f, +1.0f,
		-1.0f, -1.0f
	};
	vbo.bufferData(sizeof(data), data);
	gfx.setAttributefv("position", 2, 0, 0);
	
	// This stutters on ATI HD 6850 for some reason!

	for (int i = 0; i < num_lights; ++i)
	{
		// Yeah this is pretty bad for parallelism!
		gfx.setUniform("light_p", light_p[i]);
		gfx.setUniform("light_d", light_d[i]);
		gfx.setUniform("light_r", light_r[i]);
		gfx.setUniform("light_s", light_s[i]);
		gfx.setUniform("model", light_m[i] * inverse_view_r);
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		gfx.drawQuad(-1.0f, -1.0f, 2.0f, 2.0f);
	}

	vbo.dispose();
	vbo.unbind();
	gfx.endCustomShader();
}