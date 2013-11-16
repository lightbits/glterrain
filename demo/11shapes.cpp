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
#include <graphics/vertexarray.h>
#include <graphics/spritebatch.h>
#include <graphics/bufferedmesh.h>
#include <graphics/framebuffer.h>
using namespace graphics;

int main()
{
	const int windowWidth = 640;
	const int windowHeight = 480;
	if(!gl::createContext("Shapes", 300, 100, windowWidth, windowHeight, 24, 8, 8, false))
		gl::shutdown("Failed to create context");

	Shader 
		defaultVS(GL_VERTEX_SHADER),
		defaultFS(GL_FRAGMENT_SHADER),
		fboVS(GL_VERTEX_SHADER),
		fboFS(GL_FRAGMENT_SHADER);

	if(!defaultVS.loadFromFile("data/shaders/simple.vert") ||
		!defaultFS.loadFromFile("data/shaders/simple.frag") ||
		!fboVS.loadFromFile("data/shaders/fbopass.vert") ||
		!fboFS.loadFromFile("data/shaders/fbopass.frag"))
		gl::shutdown("Failed to load resources");

	Program 
		program0,
		program1;

	program0.create();
	program0.linkAndCheckStatus(defaultVS, defaultFS);

	program1.create();
	program1.linkAndCheckStatus(fboVS, fboFS);

	ProgramLayout program0Layout;
	program0Layout.setAttrib("position",	program0.getAttribLocation("position"));
	program0Layout.setAttrib("color",		program0.getAttribLocation("color"));
	program0Layout.setUniform("projection", program0.getUniformLocation("projection"));
	program0Layout.setUniform("model",		program0.getUniformLocation("model"));
	program0Layout.setUniform("view",		program0.getUniformLocation("view"));

	ProgramLayout program1Layout;
	program1Layout.setAttrib("position",	program1.getAttribLocation("position"));
	program1Layout.setAttrib("texel",		program1.getAttribLocation("texel"));
	program1Layout.setUniform("tex",		program1.getUniformLocation("tex"));

	TriMesh mesh0;
	mesh0 = TriMesh::genUnitColoredCube();

	// Whatever-array-object
	VertexArray vao;
	vao.create();
	vao.bind();

	BufferedMesh mesh0Buffer;
	mesh0Buffer.create(mesh0, program0Layout);

	// Data for a fullscreen quad
	GLfloat postEffectVertexData[] = {
	//	x		y		u		v
		-1.0f, -1.0f,	0.0f, 0.0f,
		-1.0f, +1.0f,	0.0f, 1.0f,
		+1.0f, +1.0f,	1.0f, 1.0f,
		+1.0f, -1.0f,	1.0f, 0.0f
	};

	GLushort postEffectIndexData[] = {
		0, 1, 2, 
		2, 3, 0
	};

	BufferObject postEffectVbo, postEffectIbo;
	postEffectVbo.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(GLfloat)*4*4, postEffectVertexData);
	postEffectIbo.create(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(GLushort)*3*2, postEffectIndexData);

	Texture texture;
	texture.create2d(0, GL_RGB, windowWidth, windowHeight, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	texture.setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	// Render buffer handle
	//GLuint rbf;

	//// Generate render buffer
	//glGenRenderbuffers(1, &rbf);
	//glBindRenderbuffer(GL_RENDERBUFFER, rbf);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, windowWidth, windowHeight);

	//GLuint fbo;
	//glGenFramebuffers(1, &fbo);
	//glBindFramebuffer(GL_FRAMEBUFFER, fbo);


	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0, 1.0);
	glClearDepth(1.0);

	// Enable culling
	/*glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);*/

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
		modelMatrix.rotateY(time);
		modelMatrix.rotateX(0.3f);
		program0.uniform(program0Layout.getUniformLoc("model"), modelMatrix.top());
		mesh0Buffer.draw();
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
	mesh0Buffer.dispose();
	vao.dispose();
	gl::shutdown();
}