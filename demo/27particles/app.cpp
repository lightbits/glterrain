/*
See http://open.gl/feedback
*/

#include "app.h"
#include <common/noise.h>
#define GLSL(src) "#version 150 core\n" #src

const int NUM_POINTS = 100000;

const char *vertex_shader_src = GLSL(
	in vec2 position;
	in vec2 velocity;

	uniform vec2 mouse_pos;
	uniform float dt;
	
	out vec2 out_position;
	out vec2 out_velocity;

	void main() {
		float dx = 1.0 * ((-1.0 + 2.0 * mouse_pos.x / 720.0) - position.x);
		float dy = 1.0 * ((+1.0 - 2.0 * mouse_pos.y / 480.0) - position.y);
		vec2 a = vec2(dx, dy) / sqrt(dx * dx + dy * dy);
		out_velocity = velocity + a * dt;
		out_position = position + velocity * dt;
		if (position.x < -1.0 || position.x > 1.0)
		{
			out_velocity.x *= -0.4;
			out_position.x = max(min(position.x, 1.0), -1.0);
		}
			
		if (position.y < -1.0 || position.y > 1.0)
		{
			out_velocity.y *= -0.4;
			out_position.y = max(min(position.y, 1.0), -1.0);
		}
		gl_Position = vec4(out_position, 0.0, 1.0);
	}
);

const char *fragment_shader_src = GLSL(
	out vec4 out_color;
	void main() {
		out_color = vec4(0.3, 0.35, 0.73, 1.0);
	}
);

VertexArray vao;
BufferObject vbo, tbo;
ShaderProgram program;
float *vertices;

bool load()
{
	if (!program.loadFromSource(vertex_shader_src, fragment_shader_src))
		return false;

	// Specify transform feedback buffer input
	const char *feedback_varyings[] = { "out_position", "out_velocity" };
	glTransformFeedbackVaryings(program.getHandle(), 2, feedback_varyings, GL_INTERLEAVED_ATTRIBS);

	if (!program.linkAndCheckStatus())
		return false;
	return true;
}

void free()
{
	delete[] vertices;
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{	
	gfx.beginCustomShader(program);
	vao.create();
	vao.bind();

	// Create a buffer with some input data
	vertices = new float[NUM_POINTS * 4];
	for (int i = 0; i < NUM_POINTS; ++i)
	{
		vertices[i * 4] = frand() * 2.0f - 1.0f;
		vertices[i * 4 + 1] = frand() * 2.0f - 1.0f;
		vertices[i * 4 + 2] = 0.1 * (frand() * 2.0f - 1.0f);
		vertices[i * 4 + 3] = 0.1 * (frand() * 2.0f - 1.0f);
	}
	vbo.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	vbo.bind();
	vbo.bufferData(NUM_POINTS * 4 * sizeof(float), vertices);
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("velocity", 2, 4, 2);

	// Create buffer to hold output data
	tbo.create(GL_ARRAY_BUFFER, GL_STATIC_READ);
	tbo.bind();
	tbo.bufferData(NUM_POINTS * 4 * sizeof(float), nullptr);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	// Get mouse position
	int x, y;
	ctx.getMousePos(&x, &y);

	// Bind transform feedback buffer
	tbo.bind();
	glBindBufferBase(
		GL_TRANSFORM_FEEDBACK_BUFFER, 
		0, // Index of the output variable 
		tbo.getHandle());

	// Enter transform feedback mode
	glBeginTransformFeedback(GL_POINTS);

	gfx.setUniform("dt", dt);
	gfx.setUniform("mouse_pos", vec2(x, y));

	// "Draw"
	glDrawArrays(GL_POINTS, 0, NUM_POINTS);

	// End mode and make sure the rendering operation actually finished
	glEndTransformFeedback();
	glFlush();

	glEnable(GL_RASTERIZER_DISCARD);

	// Get the results
	glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, NUM_POINTS * 4 * sizeof(float), vertices);

	// Upload back to vbo?
	vbo.bind();
	vbo.bufferSubData(0, NUM_POINTS * 4 * sizeof(float), vertices);
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.setBlendState(BlendStates::Additive);
	glDisable(GL_RASTERIZER_DISCARD);
	gfx.setClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	gfx.clearColorAndDepth();
	glPointSize(1.0f);
	gfx.drawVertexBuffer(GL_POINTS, NUM_POINTS);
}