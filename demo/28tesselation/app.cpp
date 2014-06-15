#include "app.h"
#define GLSL(src) "#version 150 core\n" #src

const char *vertex_shader_src = GLSL(
	in vec2 position;
	in vec3 color;
	in float sides;

	out vec3 v_color; // -> geometry shader
	out float v_sides; // -> geometry shader

	void main() 
	{
		v_color = color;
		v_sides = sides;
		gl_Position = vec4(position, 0.0, 1.0);
	}
);

const char *fragment_shader_src = GLSL(
	in vec3 f_color;

	out vec4 out_color;

	void main()
	{
		out_color = vec4(f_color, 1.0);
	}
);

const char *geometry_shader_src = GLSL(
	layout(points) in;
	layout(line_strip, max_vertices = 64) out;

	in vec3 v_color[];
	in float v_sides[];

	out vec3 f_color; // Output to fragment shader

	const float PI = 3.1415926;

	void main()
	{
		f_color = v_color[0];

		// Safe: Floats can represent small integers exactly
		for (int i = 0; i <= v_sides[0]; ++i)
		{
			float t = i * 2.0 * PI / v_sides[0];
			gl_Position = gl_in[0].gl_Position;
			gl_Position.x += 0.25 * cos(t);
			gl_Position.y += 0.375 * sin(t);
			EmitVertex();
		}
		EndPrimitive();
	}
);

ShaderProgram shader;
VertexArray vao;
BufferObject vbo;

bool load()
{
	if (!shader.loadFromSource(vertex_shader_src, fragment_shader_src, geometry_shader_src))
		return false;

	if (!shader.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{
	shader.dispose();
	vao.dispose();
	vbo.dispose();
}

void init(Renderer &gfx, Context &ctx)
{	
	const float vertices[] = {
		// Position		Color				Sides
		-0.5f, -0.5f,	1.0f, 0.5f, 0.4f,	10.0f,
		+0.5f, -0.5f,	0.4f, 1.0f, 0.5f,	4.0f,
		+0.5f, +0.5f,	0.5f, 0.4f, 1.0f,	16.0f,
		-0.5f, +0.5f,	1.0f, 0.4f, 1.0f,	32.0f
	};

	vbo.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	vbo.bind();
	vbo.bufferData(sizeof(vertices), vertices);

	// Create vao to hold buffer bindings
	vao.create();
	vao.bind();
	shader.setAttributefv("position", 2, 6, 0);
	shader.setAttributefv("color", 3, 6, 2);
	shader.setAttributefv("sides", 1, 6, 5);

	gfx.beginCustomShader(shader);
}

void update(Renderer &gfx, Context &ctx, double dt)
{

}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.setClearDepth(1.0);
	gfx.setClearColor(0.2f, 0.2f, 0.2f);
	gfx.clearColorAndDepth();
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setCullState(CullStates::CullNone);

	gfx.drawVertexBuffer(GL_POINTS, 4);
}