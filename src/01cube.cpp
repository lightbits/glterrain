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

#include <graphics/opengl.h>
#include <graphics/color.h>
#include <graphics/texture.h>
#include <graphics/trimesh.h>
#include <graphics/program.h>
#include <graphics/bufferobject.h>
#include <graphics/vertexformat.h>
#include <graphics/spritebatch.h>
#include <graphics/bufferedmesh.h>
using namespace graphics;

int main()
{
	if(!gl::createContext("Cube", 300, 100, 640, 480, 24, 8, 8, false))
		gl::shutdown("Failed to create context");

	Shader 
		defaultVS(GL_VERTEX_SHADER),
		defaultFS(GL_FRAGMENT_SHADER);

	if(!defaultVS.loadFromFile("data/shaders/simple.vert") ||
		!defaultFS.loadFromFile("data/shaders/simple.frag"))
		gl::shutdown("Failed to load resources");

	Program program0;

	program0.create();
	program0.linkAndCheckStatus(defaultVS, defaultFS);

	ProgramLayout program0Layout;

	program0Layout.setAttrib("position",	program0.getAttribLocation("position"));
	program0Layout.setAttrib("color",	program0.getAttribLocation("color"));
	program0Layout.setUniform("projection", program0.getUniformLocation("projection"));
	program0Layout.setUniform("model",		program0.getUniformLocation("model"));
	program0Layout.setUniform("view",		program0.getUniformLocation("view"));

	TriMesh cubeMesh = TriMesh::genUnitColoredCube();
	BufferedMesh cubeBuffer;
	cubeBuffer.create(cubeMesh, program0Layout);

	// Whatever-array-object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0, 1.0);
	glClearDepth(1.0);

	// Enable culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	mat4 perspectiveMatrix = glm::perspective(45.0f, 640.0f / 480.0f, 0.05f, 50.0f);

	Timer timer;
	timer.start();
	double renderTime = 0.0;

	while(glfwGetWindowParam(GLFW_OPENED) == GL_TRUE)
	{
		timer.step();
		float time = timer.getElapsedTime();

		double renderStart = timer.getElapsedTime();
		glClearColor(0.55f, 0.45f, 0.45f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		MatrixStack viewMatrix;
		MatrixStack modelMatrix;
		viewMatrix.push();
		viewMatrix.translate(0.0f, 0.0f, -3.0f);

		program0.use();
		program0.uniform(program0Layout.getUniformLoc("projection"), perspectiveMatrix);
		program0.uniform(program0Layout.getUniformLoc("view"), viewMatrix.top());

		modelMatrix.push();
		modelMatrix.rotateX(-0.58f);
		modelMatrix.rotateY(time);
		program0.uniform(program0Layout.getUniformLoc("model"), modelMatrix.top());
		cubeBuffer.draw();
		modelMatrix.pop();

		program0.unuse();
		viewMatrix.pop();

		glfwSwapBuffers();
		renderTime = timer.getElapsedTime() - renderStart;
		if(renderTime < 0.013)
			glfwSleep(0.013 - renderTime);

		GLenum error = glGetError();
		if(error != GL_NO_ERROR)
		{
			std::cerr<<gl::getErrorMessage(error)<<"...";
			std::cin.get();
			glfwCloseWindow();
		}
	}

	defaultVS.dispose();
	defaultFS.dispose();
	program0.dispose();
	cubeBuffer.dispose();
	glDeleteVertexArrays(1, &vao);
	gl::shutdown();
}