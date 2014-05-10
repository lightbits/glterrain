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
		out_color = vec4(0.1, 0.17, 1.4, 1.0) * 0.01;
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
	// We need two, because one will be used for computation
	// while the other is used for rendering, and vice versa.
	glGenTransformFeedbacks(2, feedback);

	// Set the first feedback object as active
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[0]);

	// Bind the first vbo to it, and upload some data
	vbo[0].bind();
	vbo[0].bufferData(NUM_POINTS * 4 * sizeof(float), vertices);

	// This setus up the binding against the current feedback object
	// The 0 can be viewed as the offset into stride of the feedback buffer
	// that the vbo will get data from. In this case, all the data is
	// interleaved into the same buffer, so the index is always 0.
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo[0].getHandle());

	// Likewise, but now the other buffer.
	// Note that we pass null, instead of the vertex data for this buffer,
	// as we use vbo[0] as the initial input (and then alternate between
	// vbo[1] and vbo[0])
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[1]);
	vbo[1].bind();
	vbo[1].bufferData(NUM_POINTS * 4 * sizeof(float), nullptr);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo[1].getHandle());
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	
}

int bufferIndexA = 0;
int bufferIndexB = 1;

bool first = true;

void render(Renderer &gfx, Context &ctx, double dt)
{
	// Get mouse position
	int x, y;
	ctx.getMousePos(&x, &y);

	// Transform feedback pass (no rendering)
	gfx.beginCustomShader(program);
	glEnable(GL_RASTERIZER_DISCARD);

	// The input buffer is this vbo - we bind it as usual
	vbo[bufferIndexA].bind();

	// The output buffer is the other vbo.
	// But to specify that it should be the output target, we need
	// to bind the feedback object that holds the binding to the vbo.
	// (A feedback object holds information about which VBO is bound to it)
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[bufferIndexB]); // Brings along the output buffer

	// From this line on, all vertex shader outputs are redirected to the bound 
	// feedback's vbo, until the line glEndTransformFeedback
	glBeginTransformFeedback(GL_POINTS);

	// Prepare rendering
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("velocity", 2, 4, 2);
	gfx.setUniform("dt", dt);
	gfx.setUniform("mouse_pos", vec2(x, y));

	// Now we have two choices:
	// a) call glDrawArrays(...), specifying the number of points to be drawn
	// b) call glDrawTransformFeedback(), not doing so!
	// The way b) works is that we instead give it a feedback object, from which
	// it can deduce how many vertices need to be drawn based on how many were
	// draw in the past.

	// To do b) though, we would need to do a) atleast once at first,
	// so that there is some initial info about the vertex count.

	// Draw the vertex buffer and have the number of vertices to draw be automatically
	// deduced by the previous buffer
	/*if (first) {
		gfx.drawVertexBuffer(GL_POINTS, NUM_POINTS);
		first = false;
	}
	else {
		glDrawTransformFeedback(GL_POINTS, feedback[bufferIndexA]);
	}*/

	gfx.drawVertexBuffer(GL_POINTS, NUM_POINTS);
	glEndTransformFeedback();

	// Make sure it's done
	glFlush();

	// Render pass
	gfx.setBlendState(BlendStates::Additive);
	glDisable(GL_RASTERIZER_DISCARD);
	gfx.setClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	gfx.clearColorAndDepth();

	// Because we bound the transform object of index bufferIndexB, 
	// its corresponding vbo now holds the output. We bind the buffer
	// and use it to render.
	vbo[bufferIndexB].bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("velocity", 2, 4, 2);
	gfx.setUniform("dt", dt);
	gfx.setUniform("mouse_pos", vec2(x, y));

	// Could use glDrawTransformFeedback here as well, specifying feedback[bufferIndexB].
	gfx.drawVertexBuffer(GL_POINTS, NUM_POINTS);
	gfx.endCustomShader();

	// Swap buffer indices
	bufferIndexA = bufferIndexB;
	bufferIndexB = (bufferIndexB + 1) % 2;
}