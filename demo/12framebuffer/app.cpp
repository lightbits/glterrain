#include "app.h"
using namespace transform;

VertexArray vao;
ShaderProgram 
	shader_dif,
	shader_tex,
	shader_nor;

Model model;
MeshBuffer mesh_buffer;

RenderTexture rt_left;
RenderTexture rt_top;
RenderTexture rt_front;
RenderTexture rt_free;

bool load()
{
	Mesh mesh;
	if (!mesh.loadFromFile("./demo/12framebuffer/teapot.obj"))
		return false;
	mesh.clearColors();
	mesh.clearTexels();
	mesh_buffer.create(mesh);
	model = Model(mesh_buffer);

	if (!shader_dif.loadFromFile("./demo/12framebuffer/diffuse") ||
		!shader_tex.loadFromFile("./demo/12framebuffer/texture") ||
		!shader_nor.loadFromFile("./demo/12framebuffer/normal"))
		return false;

	if (!shader_dif.linkAndCheckStatus() ||
		!shader_tex.linkAndCheckStatus() ||
		!shader_nor.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();

	rt_free.create(ctx.getWidth(), ctx.getHeight());
	rt_front.create(ctx.getWidth(), ctx.getHeight());
	rt_top.create(ctx.getWidth(), ctx.getHeight());
	rt_left.create(ctx.getWidth(), ctx.getHeight());

	gfx.setClearDepth(1.0);
	gfx.setClearColor(0.2f, 0.2f, 0.3f);
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setCullState(CullStates::CullCounterClockwise);
}

void update(Renderer &gfx, Context &ctx, double dt)
{

}

void drawModel(Renderer &gfx, Context &ctx)
{
	model.transform.push();
	model.transform.rotateY(ctx.getElapsedTime());
	model.transform.scale(0.1f);
	model.draw();
	model.transform.pop();
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.setClearColor(Color::fromHex(0x293A56FF));
	gfx.clearColorAndDepth();
	float ar = ctx.getWidth() / (float)ctx.getHeight();

	// Orthographic views
	gfx.beginCustomShader(shader_nor);
	gfx.setUniform("projection", glm::ortho(-1.0f * ar, 1.0f * ar, -1.0f, 1.0f, 0.1f, 10.0f));

	rt_left.begin();
	gfx.clearColorAndDepth();
	gfx.setUniform("view", translate(0.0f, 0.0f, -5.0f) * rotateY(PI / 2.0f));
	drawModel(gfx, ctx);
	rt_left.end();

	gfx.setRasterizerState(RasterizerStates::LineBoth);

	rt_top.begin();
	gfx.clearColorAndDepth();
	gfx.setUniform("view", translate(0.0f, 0.0f, -5.0f) * rotateX(-PI / 2.0f));
	drawModel(gfx, ctx);
	rt_top.end();

	rt_front.begin();
	gfx.clearColorAndDepth();
	gfx.setUniform("view", translate(0.0f, 0.0f, -5.0f));
	drawModel(gfx, ctx);
	rt_front.end();

	// Perspective view
	gfx.beginCustomShader(shader_dif);
	gfx.setRasterizerState(RasterizerStates::FillBoth);
	gfx.setUniform("projection", glm::perspective(45.0f, ctx.getWidth() / (float)ctx.getHeight(), 0.05f, 10.0f));

	rt_free.begin();
	gfx.clearColorAndDepth();
	gfx.setUniform("view", translate(0.0f, 0.0f, -5.0f) * rotateX(-PI / 4.0f));
	drawModel(gfx, ctx);
	rt_free.end();

	gfx.beginCustomShader(shader_tex);
	gfx.setClearColor(Color::fromHex(0x4B5C74FF));
	gfx.clearColorAndDepth();
	gfx.setUniform("tex", 0);

	rt_free.bindTexture();
	gfx.drawTexQuad(0.01f, -1.0f, 1.0f, 1.0f);

	rt_top.bindTexture();
	gfx.drawTexQuad(0.01f, 0.01f, 1.0f, 1.0f);

	rt_left.bindTexture();
	gfx.drawTexQuad(-1.0f, -1.0f, 1.0f, 1.0f);

	rt_front.bindTexture();
	gfx.drawTexQuad(-1.0f, 0.01f, 1.0f, 1.0f);

	rt_front.unbindTexture();
	gfx.endCustomShader();
}