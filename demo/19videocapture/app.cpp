#include "app.h"
#include <video/videocapture.h>
#include <sstream>

VertexArray vao;
ShaderProgram shader;
Model cube;
MeshBuffer cube_buffer;

bool load()
{
	if (!shader.loadAndLinkFromFile("./demo/19videocapture/simple"))
		return false;

	return true;
}

void free()
{					  
	VideoCapture::dump("./video");
	cube_buffer.dispose();
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{	
	VideoCapture::reserve(ctx.getWidth(), ctx.getHeight(), 25, 2);
	vao.create();
	vao.bind();

	cube_buffer.create(Mesh::genUnitCube(true, false, true));
	cube = Model(cube_buffer);
}

void update(Renderer &gfx, Context &ctx, double dt)
{

}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.setClearDepth(1.0);
	gfx.setClearColor(Color::fromHex(0x7f7f7fff));
	gfx.clearColorAndDepth();
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setCullState(CullStates::CullNone);

	gfx.beginCustomShader(shader);
	gfx.setUniform("view", transform::translate(0.0f, 0.0f, -3.5f));
	gfx.setUniform("projection", glm::perspective(45.0f, ctx.getWidth() / (float)ctx.getHeight(), 0.05f, 10.0f));
	cube.transform = transform::rotateY(ctx.getElapsedTime());
	cube.draw();
	gfx.endCustomShader();	

	VideoCapture::record(dt);
}