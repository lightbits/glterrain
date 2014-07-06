#include "app.h"
using namespace transform;

VertexArray vao;
ShaderProgram shader;
MeshBuffer quad;
mat4 
	mat_projection, 
	mat_view;

bool load()
{
	if (!shader.loadFromFile("./demo/36blending/quad"))
		return false;

	if (!shader.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{
	quad.dispose();
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{	
	vao.create();
	vao.bind();

	quad = MeshBuffer(Mesh::genPlane(2.0f, 2.0f));
	mat_projection = glm::perspective(PI / 4.0f, ctx.getWidth() / (float)ctx.getHeight(), 0.1f, 10.0f);
	mat_view = translate(0.0f, 0.0f, -2.0f) * rotateX(-0.35f);

	gfx.beginCustomShader(shader);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("projection", mat_projection);
}

void update(Renderer &gfx, Context &ctx, double dt)
{

}

void render(Renderer &gfx, Context &ctx, double dt)
{
	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDepthRange(0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);

	glEnable(GL_BLEND);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gfx.setUniform("model", scale(1.0f));
	gfx.setUniform("color", vec4(1.0f, 0.4f, 0.4f, 1.0f));
	quad.draw();

	glBlendFuncSeparate(
		GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, 
		GL_ONE_MINUS_DST_ALPHA, GL_ONE);

	gfx.setUniform("model", translate(0.0f, 0.1f, 0.0f) * scale(0.5f, 1.0f, 1.0f) * rotateX(-1.0f));
	gfx.setUniform("color", vec4(0.4f, 0.4f, 1.0f, 0.5f));
	quad.draw();
}