#include "app.h"

VertexArray vao;
ShaderProgram shader;
Model cube;
MeshBuffer cube_buffer;

bool load()
{
	if (!shader.loadFromFile("./demo/01cube/simple"))
		return false;

	if (!shader.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{
	cube_buffer.dispose();
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();
	
	Mesh cube_mesh = Mesh::genUnitCube(true, false);
	cube_buffer.create(cube_mesh);
	cube = Model(cube_buffer);
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
	gfx.setCullState(CullStates::CullCounterClockwise);

	gfx.beginCustomShader(shader);
	gfx.setUniform("view", mat4(1.0f));
	gfx.setUniform("projection", glm::perspective(45.0f, ctx.getWidth() / (float)ctx.getHeight(), 0.05f, 10.0f));
	cube.transform.push();
	cube.transform.translate(0.0f, 0.0f, -3.5f);
	cube.transform.rotateY(ctx.getElapsedTime());
	cube.draw();
	cube.transform.pop();
	gfx.endCustomShader();
}