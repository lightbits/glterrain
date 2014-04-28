/*
See http://open.gl/feedback
*/

#include "app.h"
#include <common/noise.h>
#define GLSL(src) "#version 150 core\n" #src

const int NUM_POINTS = 1000000;

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
		float dist = sqrt(dx * dx + dy * dy);
		vec2 a = vec2(dx, dy) / dist;
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
		out_color = vec4(0.1, 0.17, 0.9, 1.0) * 0.01;
		out_color.rgb = pow(out_color.rgb, vec3(1.0 / 2.2));
	}
);

VertexArray vao;
BufferObject vbo[2];
GLuint feedback[2];
ShaderProgram program;
float *vertices;

bool load()
{
	if (!program.loadFromSource(vertex_shader_src, fragment_shader_src))
		return false;

	// Specify transform feedback buffer input
	const char *varyings[] = { "out_position", "out_velocity" };
	glTransformFeedbackVaryings(program.getHandle(), 2, varyings, GL_INTERLEAVED_ATTRIBS);

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


	vbo[0].create(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
	vbo[1].create(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);

	// Create transform feedback objects
	glGenTransformFeedbacks(2, feedback);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[0]);
	vbo[0].bind();
	vbo[0].bufferData(NUM_POINTS * 4 * sizeof(float), vertices);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo[0].getHandle());

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[1]);
	vbo[1].bind();
	vbo[1].bufferData(NUM_POINTS * 4 * sizeof(float), vertices);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo[1].getHandle());
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	
}

int bufferIndexA = 0;
int bufferIndexB = 1;

void render(Renderer &gfx, Context &ctx, double dt)
{
	// Get mouse position
	int x, y;
	ctx.getMousePos(&x, &y);

	// Transform feedback pass (no rendering)
	gfx.beginCustomShader(program);
	glEnable(GL_RASTERIZER_DISCARD);

	vbo[bufferIndexA].bind(); // Input
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[bufferIndexB]); // Brings along the output buffer
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("velocity", 2, 4, 2);
	glBeginTransformFeedback(GL_POINTS);
	// From now on, all vertex shader outputs are redirected to the bound feedback's vbo

	gfx.setUniform("dt", dt);
	gfx.setUniform("mouse_pos", vec2(x, y));
	gfx.drawVertexBuffer(GL_POINTS, NUM_POINTS);
	glEndTransformFeedback();
	glFlush();

	// Render pass
	gfx.setBlendState(BlendStates::Additive);
	glDisable(GL_RASTERIZER_DISCARD);
	gfx.setClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	gfx.clearColorAndDepth();
	vbo[bufferIndexB].bind(); // Use output from transform feedback as input
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("velocity", 2, 4, 2);
	gfx.setUniform("dt", dt);
	gfx.setUniform("mouse_pos", vec2(x, y));
	gfx.drawVertexBuffer(GL_POINTS, NUM_POINTS);
	gfx.endCustomShader();

	// Swap buffer indices
	bufferIndexA = bufferIndexB;
	bufferIndexB = (bufferIndexB + 1) % 2;
}