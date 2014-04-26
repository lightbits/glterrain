/*
Uses vertex array objects to store vertex buffer bindings and attribute pointer thingies.
*/

#include "app.h"

ShaderProgram shader;
VertexArray vao;
BufferObject vbo;

bool load()
{
	if (!shader.loadFromFile("./demo/04vao/simple"))
		return false;

	if (!shader.linkAndCheckStatus())
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
	// Create the vertex buffer that will be updated with vertices during runtime
	vbo.create(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
	vbo.bind();
	vbo.bufferData(sizeof(float) * 6 * 7, NULL);
	vbo.unbind();

	// Create VAO to hold attribute bindings
	vao.create();
	vao.bind();
	vbo.bind();
	shader.setAttributefv("position", 3, 7, 0);
	shader.setAttributefv("color", 4, 7, 3);
	vbo.unbind();
	vao.unbind();
}

void update(Renderer &gfx, Context &ctx, double dt)
{

}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.setClearDepth(1.0);
	gfx.setClearColor(0.2f, 0.2f, 0.3f);
	gfx.clearColorAndDepth();
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setCullState(CullStates::CullNone);

	gfx.beginCustomShader(shader);
	gfx.setUniform("view", mat4(1.0f));
	gfx.setUniform("projection", glm::perspective(45.0f, ctx.getWidth() / (float)ctx.getHeight(), 0.05f, 10.0f));
	gfx.setUniform("model", transform::translate(0.0f, 0.0f, -3.5f) * transform::rotateY(ctx.getElapsedTime()));

	vao.bind();
	vbo.bind(); // This is necessary
	// These aren't!
	//gfx.setAttributefv("position", 3, 7, 0);
	//gfx.setAttributefv("color", 4, 7, 3);

	// Update buffer with dynamic data
	float sint = sin(ctx.getElapsedTime());
	float cost = cos(ctx.getElapsedTime());
	float vertices[] = {
		-0.5f, -0.5f, 0.0f,		0.5f * sint + 0.5f, 0.5f * cost + 0.5f, 1.0f, 1.0f,
		 0.5f, -0.5f, 0.0f,		0.5f * sint + 0.5f, 0.5f * cost + 0.5f, 1.0f, 1.0f,
		 0.5f,  0.5f, 0.0f,		0.5f * sint + 0.5f, 0.5f * cost + 0.5f, 1.0f, 1.0f,

		 0.5f,  0.5f, 0.0f,		0.5f * sint + 0.5f, 0.5f * cost + 0.5f, 1.0f, 1.0f,
		-0.5f,  0.5f, 0.0f,		0.5f * sint + 0.5f, 0.5f * cost + 0.5f, 1.0f, 1.0f,
		-0.5f, -0.5f, 0.0f,		0.5f * sint + 0.5f, 0.5f * cost + 0.5f, 1.0f, 1.0f
	};
	vbo.bufferSubData(0, sizeof(vertices), vertices);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	gfx.endCustomShader();	
}