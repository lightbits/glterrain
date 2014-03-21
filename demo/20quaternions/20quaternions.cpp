/*
http://codeflow.org/entries/2011/apr/13/advanced-webgl-part-2-sky-rendering/
*/

#include <app/glcontext.h>
#include <graphics/renderer.h>
#include <gl/shaderprogram.h>
#include <graphics/model.h>
#include <common/transform.h>
#include <camera/camera.h>
#include <graphics/spritebatch.h>

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

Camera camera;

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
	camera.setPosition(vec3(0.0f, 0.0f, -1.0f));
	ctx.setMousePos(ctx.getWidth() / 2, ctx.getHeight() / 2);
}

float y_rotation = 0.0f;

void updateCamera(Renderer &gfx, Context &ctx, double dt)
{
	if (glfwGetKey(GLFW_KEY_LCTRL))
		camera.moveDown(dt);
	else if (glfwGetKey(GLFW_KEY_SPACE))
		camera.moveUp(dt);

	if (glfwGetKey('W'))
		camera.moveForward(dt);
	else if (glfwGetKey('S'))
		camera.moveBackward(dt);

	if (glfwGetKey('A'))
		camera.moveLeft(dt);
	else if (glfwGetKey('D'))
		camera.moveRight(dt);

	int wh = ctx.getWidth() / 2;
	int hh = ctx.getHeight() / 2;
	int dx = ctx.getMouseX() - wh;
	int dy = ctx.getMouseY() - hh;
	camera.rotateHorizontal(dx * 0.4f * dt);
	camera.rotateVertical(dy * 0.4f * dt);
	ctx.setMousePos(wh, hh);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	if (glfwGetKey(GLFW_KEY_ESC))
		ctx.close();

	y_rotation += dt;
	updateCamera(gfx, ctx, dt);
}

void drawBoxMonster(Renderer &gfx, Context &ctx, double dt)
{
	cube.pushTransform();
		cube.rotateY(y_rotation);
		cube.pushTransform();
			cube.scale(0.65f, 0.2f, 0.2f);
			cube.draw();
		cube.popTransform();
		cube.pushTransform();
			cube.translate(-0.15f, 0.0f, 0.0f);
			cube.scale(0.3f, 0.14f, 0.6f);
			cube.draw();
		cube.popTransform();
		cube.pushTransform();
			cube.translate(0.325f, 0.0f, 0.0f);
			cube.rotateZ(3.1415f / 4.0f);
			cube.scale(0.141f, 0.141f, 0.2f);
			cube.draw();
		cube.popTransform();
	cube.popTransform();
}

void drawAxes(Renderer &gfx, Context &ctx, double dt)
{
	cylinder.pushTransform();
		cylinder.rotateY(y_rotation);
		cylinder.scale(0.7f, 0.7f, 0.02f);
		cylinder.draw();
	cylinder.popTransform();
	cylinder.pushTransform();
		cylinder.rotateY(y_rotation);
		cylinder.rotateX(1.54f);
		cylinder.scale(0.7f, 0.7f, 0.02f);
		cylinder.draw();
	cylinder.popTransform();
	cylinder.pushTransform();
		cylinder.rotateY(y_rotation + 1.54f);
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
	gfx.setUniform("view", camera.getViewMatrix());
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

int main()
{
	GLContext ctx;
	if (!ctx.create(VideoMode(720, 480, 24, 0, 4, 3, 1, false), "Irradiance", true, true))
		crash("Failed to open context");

	Renderer gfx;
	gfx.init(ctx);

	if (!load())
	{
		ctx.dispose();
		crash("Failed to load content");
	}

	try
	{
		init(gfx, ctx);

		int updatesPerSec = 60;
		double targetFrameTime = 1.0 / 60.0;
		double secsPerUpdate = 1.0 / double(updatesPerSec);
		double accumulator = 0.0;
		double dt = 0.0;
		double prevNow = 0.0;
		while (ctx.isOpen())
		{
			double now = glfwGetTime();
			double dt = now - prevNow;
			prevNow = now;

			accumulator += dt;
			while (accumulator >= secsPerUpdate)
			{
				update(gfx, ctx, secsPerUpdate);
				accumulator -= secsPerUpdate;
			}

			double updateTime = glfwGetTime() - now;
			now = glfwGetTime();

			render(gfx, ctx, dt);
			ctx.display();
			ctx.pollEvents();

			double renderTime = glfwGetTime() - now;
			if (renderTime < targetFrameTime)
				ctx.sleep(targetFrameTime - renderTime);

			if (checkGLErrors(std::cerr))
				ctx.close();
		}
	}
	catch (std::exception &e)
	{
		std::cerr << "An unexpected error occured: " << e.what() << std::endl;
		std::cin.get();
	}

	free();
	gfx.dispose();
	ctx.dispose();
	exit(EXIT_SUCCESS);
}