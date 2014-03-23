/*
http://antongerdelan.net/opengl/quaternions.html
*/

#include "app.h"
#include <graphics/model.h>
#include <graphics/spritebatch.h>
#include <common/transform.h>
#include <common/typedefs.h>
#include <common/quaternion.h>
#include <app/log.h>
#include "../fpcamera.h"

MeshBuffer cubebuffer;
MeshBuffer inner_gridbuffer;
MeshBuffer outer_gridbuffer;
MeshBuffer cylinderbuffer;
Model cube;
Model inner_grid;
Model outer_grid;
Model cylinder;

VertexArray vao;

ShaderProgram shader_default;

Font font;
SpriteBatch spritebatch;

bool load()
{
	Mesh cube_mesh = Mesh::genUnitColoredCube();
	cubebuffer.create(cube_mesh);
	cube = Model(cubebuffer);

	inner_gridbuffer.create(Mesh::genUnitGrid(Color(0.4f), 24));
	outer_gridbuffer.create(Mesh::genUnitGrid(Color(1.0f), 8));
	inner_grid = Model(inner_gridbuffer);
	outer_grid = Model(outer_gridbuffer);

	Mesh cylinder_mesh = Mesh::genUnitCylinder(Colors::White, 64);
	cylinderbuffer.create(cylinder_mesh);
	cylinder = Model(cylinderbuffer);

	spritebatch.create();

	if (!font.loadFromFile("data/fonts/proggytinyttsz_8x12.png"))
		return false;

	if (!shader_default.loadFromFile("demo/20quaternions/default"))
		return false;

	if (!shader_default.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{
	cubebuffer.dispose();
	inner_gridbuffer.dispose();
	outer_gridbuffer.dispose();
}

void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();
	resetCamera(-PI, 0.0f, vec3(0.0f, 0.0f, -2.0f));
}

float roll = 0.0f;
float pitch = 0.0f;

void update(Renderer &gfx, Context &ctx, double dt)
{
	if (ctx.isKeyPressed(SDL_SCANCODE_LEFT))
		roll += dt;
	else if (ctx.isKeyPressed(SDL_SCANCODE_RIGHT))
		roll -= dt;

	if (ctx.isKeyPressed(SDL_SCANCODE_UP))
		pitch += dt;
	else if (ctx.isKeyPressed(SDL_SCANCODE_DOWN))
		pitch -= dt;
	updateCamera(gfx, ctx, dt);
}

void drawBoxMonster(Renderer &gfx, Context &ctx, double dt)
{
	/* Combining rotations around the axes is done by multiplying quaternions.
	The cool thing is that axis of rotation is naturally updated when doing this.
	For example: rotating around the z-axis will change the "y-axis" to be upwards
	relative to the object! This produces more intuitive rotation than by Euler angles? */

	cube.pushTransform();
		//quat q = quaternion(roll, vec3(0, 1, 0));
		//quat r = quaternion(pitch, vec3(1, 0, 0));
		//cube.multiply(q * r);

		quat q = quat(vec3(pitch, roll, 0.0f));
		cube.multiply(q);

		//quaternion q(roll, vec3(0, 1, 0));
		//quaternion r(pitch, vec3(0, 0, 1));
		//cube.multiply((q * r).getMatrix());
		cube.pushTransform();
			cube.scale(0.2f, 0.2f, 0.65f);
			cube.draw();
		cube.popTransform();
		cube.pushTransform();
			cube.translate(0.0f, 0.0f, -0.15f);
			cube.scale(0.6f, 0.14f, 0.3f);
			cube.draw();
		cube.popTransform();
		cube.pushTransform();
			cube.translate(0.0f, 0.0f, 0.325f);
			cube.scale(0.2f, 0.141f, 0.141f);
			cube.rotateX(PI / 4.0f);
			cube.draw();
		cube.popTransform();
	cube.popTransform();
}

void drawAxes(Renderer &gfx, Context &ctx, double dt)
{
	cylinder.pushTransform();
		cylinder.scale(0.7f, 0.7f, 0.02f);
		cylinder.draw();
	cylinder.popTransform();
	cylinder.pushTransform();
		cylinder.rotateX(1.54f);
		cylinder.scale(0.7f, 0.7f, 0.02f);
		cylinder.draw();
	cylinder.popTransform();
	cylinder.pushTransform();
		cylinder.scale(0.7f, 0.7f, 0.02f);
		cylinder.draw();
	cylinder.popTransform();
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.setClearColor(0.23f, 0.23f, 0.23f);
	gfx.setClearDepth(1.0);
	gfx.setCullState(CullStates::CullCounterClockwise);
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setBlendState(BlendStates::AlphaBlend);
	gfx.clearColorAndDepth();

	gfx.beginCustomShader(shader_default);
	gfx.setUniform("view", getCameraView());
	gfx.setUniform("projection", glm::perspective(45.0f, 720 / 480.0f, 0.05f, 10.0f));
	inner_grid.draw();
	outer_grid.draw();
	drawBoxMonster(gfx, ctx, dt);
	gfx.endCustomShader();

	spritebatch.begin();
	spritebatch.setFont(font);
	spritebatch.drawString("Hello World!", vec2(5.0f, 5.0f), Colors::White),
	spritebatch.end();
}