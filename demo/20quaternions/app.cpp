/*
http://antongerdelan.net/opengl/quaternions.html
*/

#include "app.h"
#include <graphics/model.h>
#include <graphics/spritebatch.h>
#include <common/transform.h>
#include <common/typedefs.h>
#include <app/log.h>
#include <camera/freecamera.h>

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

FreeCamera camera;

bool load()
{
	Mesh cube_mesh = Mesh::genUnitCube(true, false, true);
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
	camera.reset(-PI, 0.0f, vec3(0.0f, 0.0f, -2.0f));
}

float roll = 0.0f;
float pitch = 0.0f;
float yaw = 0.0f;

void update(Renderer &gfx, Context &ctx, double dt)
{
	if (ctx.isKeyPressed(SDL_SCANCODE_LEFT))
		roll += dt;
	else if (ctx.isKeyPressed(SDL_SCANCODE_RIGHT))
		roll -= dt;

	if (ctx.isKeyPressed(SDL_SCANCODE_UP))
		pitch -= dt;
	else if (ctx.isKeyPressed(SDL_SCANCODE_DOWN))
		pitch += dt;

	if (ctx.isKeyPressed(SDL_SCANCODE_Q))
		yaw -= dt;
	else if (ctx.isKeyPressed(SDL_SCANCODE_E))
		yaw += dt;

	camera.update(gfx, ctx, dt);
}

void drawBoxMonster(Renderer &gfx, Context &ctx, double dt)
{
	/* Combining rotations around the axes is done by multiplying quaternions.
	The cool thing is that axis of rotation is naturally updated when doing this.
	For example: rotating around the z-axis will change the "y-axis" to be upwards
	relative to the object! This produces more intuitive rotation than by Euler angles? */

	cube.transform.push();
		quat q = quaternion(roll, vec3(0, 1, 0));
		quat r = quaternion(pitch, vec3(1, 0, 0));
		quat p = quaternion(yaw, vec3(0, 0, 1));
		cube.transform.multiply(p * r * q);

		//quat q = quat(vec3(pitch, roll, 0.0f));
		//cube.multiply(q);

		//quaternion q(roll, vec3(0, 1, 0));
		//quaternion r(pitch, vec3(0, 0, 1));
		//cube.multiply((q * r).getMatrix());
		mat4 m = glm::mat4_cast(p * r * q) * transform::translate(0, 0.1f, 0);
		gfx.setUniform("model", m);
		gfx.drawLine(vec3(0, 0, 0), vec3(0, 0.5f, 0), Color(1.0f, 0.4f, 0.4f));
		cube.transform.push();
			cube.transform.scale(0.2f, 0.2f, 0.65f);
			cube.draw();
		cube.transform.pop();
		cube.transform.push();
			cube.transform.translate(0.0f, 0.0f, -0.15f);
			cube.transform.scale(0.6f, 0.14f, 0.3f);
			cube.draw();
		cube.transform.pop();
		cube.transform.push();
			cube.transform.translate(0.0f, 0.0f, 0.325f);
			cube.transform.scale(0.2f, 0.141f, 0.141f);
			cube.transform.rotateX(PI / 4.0f);
			cube.draw();
		cube.transform.pop();
	cube.transform.pop();
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.setClearColor(0.23f, 0.23f, 0.23f);
	gfx.setClearDepth(1.0);
	gfx.setCullState(CullStates::CullClockwise);
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setBlendState(BlendStates::AlphaBlend);
	gfx.clearColorAndDepth();

	gfx.beginCustomShader(shader_default);
	gfx.setUniform("view", camera.getViewMatrix());
	gfx.setUniform("projection", glm::perspective(45.0f, 720 / 480.0f, 0.05f, 10.0f));
	inner_grid.draw();
	outer_grid.draw();
	drawBoxMonster(gfx, ctx, dt);
	gfx.endCustomShader();

	spritebatch.begin();
	spritebatch.setFont(font);
	std::string text = "Hold <e> or <q> to change yaw\nHold <left> or <right> to change roll\nHold <up> or <down> to change pitch";
	spritebatch.drawString(text, vec2(5, 5), Colors::White);
	spritebatch.end();
}