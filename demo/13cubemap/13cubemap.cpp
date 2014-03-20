#include <iostream>
#include <fstream>
#include <vector>

#include <common/helpers.h>
#include <common/text.h>
#include <common/timer.h>
#include <common/vec.h>
#include <common/matrix.h>
#include <common/matrixstack.h>

#include <camera/camera.h>

#include <gl/opengl.h>
#include <gl/texture.h>
#include <gl/program.h>
#include <gl/bufferobject.h>
#include <gl/vertexformat.h>
#include <gl/vertexarray.h>
#include <gl/bufferedmesh.h>
#include <gl/shaderprogram.h>
#include <gl/framebuffer.h>
#include <graphics/renderer.h>
#include <graphics/color.h>
#include <graphics/trimesh.h>
#include <graphics/spritebatch.h>
#include <graphics/model.h>
#include <app/glcontext.h>

Mesh generateCube()
{
	float vertices[] = {
		-10.0f,  10.0f, -10.0f,
		-10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,
  
		-10.0f, -10.0f,  10.0f,
		-10.0f, -10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f,  10.0f,
		-10.0f, -10.0f,  10.0f,
  
		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
   
		-10.0f, -10.0f,  10.0f,
		-10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f, -10.0f,  10.0f,
		-10.0f, -10.0f,  10.0f,
  
		-10.0f,  10.0f, -10.0f,
		10.0f,  10.0f, -10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		-10.0f,  10.0f,  10.0f,
		-10.0f,  10.0f, -10.0f,
  
		-10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f,  10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f,  10.0f,
		10.0f, -10.0f,  10.0f
	};

	Mesh mesh;
	mesh.addPositions((vec3*)(vertices), 36);
	return mesh;
}

void run()
{
	const int windowWidth = 640;
	const int windowHeight = 480;
	GLContext context;
	if(!context.create("Cubemap", VideoMode(windowWidth, windowHeight, 24, 8, 8, false)))
		crash("Failed to create context");

	Renderer renderer;
	renderer.init();

	ShaderProgram shaderCubemap;
	if(!shaderCubemap.loadFromFile("./demo/13cubemap/cubemap"))
		crash("Failed to load resources");
	shaderCubemap.linkAndCheckStatus();

	VertexArray vao;
	vao.create();
	vao.bind();

	Mesh cubeMesh = Mesh::genUnitColoredCube();
	MeshBuffer cubeBuffer;
	cubeBuffer.create(cubeMesh);
	Model cube(cubeBuffer);

	Mesh cubemapMesh = generateCube();
	MeshBuffer cubemapBuffer;
	cubemapBuffer.create(cubemapMesh);

	renderer.setDepthTestState(DepthTestStates::LessThanOrEqual);
	renderer.setCullState(CullStates::CullClockwise);
	renderer.setClearDepth(1.0);
	renderer.setClearColor(Color(0.55f, 0.45f, 0.45f, 1.0f));

	mat4 perspectiveMatrix = glm::perspective(45.0f, 640.0f / 480.0f, 0.05f, 50.0f);

	Timer timer;
	timer.start();
	double renderTime = 0.0;

	while(context.isOpen())
	{
		timer.step();
		float time = timer.getElapsedTime();
		double renderStart = timer.getElapsedTime();
		renderer.clearColorAndDepth();

		MatrixStack viewMatrix;
		viewMatrix.push();
		viewMatrix.translate(0.0f, 0.0f, -3.0f);

		shaderCubemap.begin();
		shaderCubemap.setUniform("projection", perspectiveMatrix);
		shaderCubemap.setUniform("view", viewMatrix.top());
		cube.pushTransform();
		cube.rotateY(time);
		cube.rotateX(time * 0.5f);
		cube.draw(GL_TRIANGLES);
		cube.popTransform();
		viewMatrix.pop();
		shaderCubemap.end();

		context.display();
		renderTime = timer.getElapsedTime() - renderStart;
		if(renderTime < 0.013)
			context.sleep(0.013 - renderTime);

		GLenum error = glGetError();
		if(error != GL_NO_ERROR)
		{
			std::cerr<<getErrorMessage(error)<<"...";
			std::cin.get();
			context.close();
		}
	}

	shaderCubemap.dispose();
	cubeBuffer.dispose();
	vao.dispose();
	exit(EXIT_SUCCESS);
}

int main()
{
	try
	{
		run();
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
		std::cin.get();
	}
	return EXIT_SUCCESS;
}