/*
http://codeflow.org/entries/2011/apr/13/advanced-webgl-part-2-sky-rendering/
*/

#include <app/glcontext.h>
#include <graphics/renderer.h>
#include <gl/shaderprogram.h>
#include <graphics/model.h>
#include <common/transform.h>

MeshBuffer meshbuffer;
Model model;

VertexArray vao;

ShaderProgram shader_default;

bool load()
{
	Mesh cube = Mesh::genUnitColoredCube();
	meshbuffer.create(cube);
	model = Model(meshbuffer);

	if (!shader_default.loadFromFile("demo/18irradiance/default"))
		return false;

	if (!shader_default.linkAndCheckStatus())
		return false;

	return true;
}

void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();
	gfx.init();
}

void update(Renderer &gfx, Context &ctx, double dt)
{

}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.setClearColor(0.21f, 0.21f, 0.21f);
	gfx.setClearDepth(1.0);
	gfx.clearColorAndDepth();
	gfx.setCullState(CullStates::CullCounterClockwise);
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);

	gfx.beginCustomShader(shader_default);
	gfx.setUniform("view", transform::translate(0.0f, 0.0f, -2.0f));
	gfx.setUniform("projection", glm::perspective(45.0f, 720 / 480.0f, 0.05f, 10.0f));
	model.rotateY(dt);
	model.rotateX(dt);
	model.draw(GL_TRIANGLES);
	gfx.endCustomShader();
}

void free()
{
	meshbuffer.dispose();
}

int main()
{
	GLContext context;
	if (!context.create(VideoMode(720, 480, 0, 0, 4, 3, 1, false), "Irradiance", true, true))
		crash("Failed to open context");

	Renderer renderer;
	renderer.init();

	if (!load())
	{
		context.dispose();
		crash("Failed to load content");
	}

	glViewport(0, 0, 720, 480);

	try
	{
		init(renderer, context);

		int updatesPerSec = 60;
		double targetFrameTime = 1.0 / 60.0;
		double secsPerUpdate = 1.0 / double(updatesPerSec);
		double accumulator = 0.0;
		double dt = 0.0;
		double prevNow = 0.0;
		while (context.isOpen())
		{
			double now = glfwGetTime();
			double dt = now - prevNow;
			prevNow = now;

			accumulator += dt;
			while (accumulator >= secsPerUpdate)
			{
				update(renderer, context, secsPerUpdate);
				accumulator -= secsPerUpdate;
			}

			double updateTime = glfwGetTime() - now;
			now = glfwGetTime();

			render(renderer, context, dt);
			context.display();
			context.pollEvents();

			double renderTime = glfwGetTime() - now;
			if (renderTime < targetFrameTime)
				context.sleep(targetFrameTime - renderTime);

			if (checkGLErrors(std::cerr))
				context.close();
		}
	}
	catch (std::exception &e)
	{
		std::cerr << "An unexpected error occured: " << e.what() << std::endl;
		std::cin.get();
	}

	std::cin.get();

	free();
	context.dispose();
	exit(EXIT_SUCCESS);
}