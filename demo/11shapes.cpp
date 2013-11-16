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
#include <app/glcontext.h>

int main()
{
	const int windowWidth = 640;
	const int windowHeight = 480;
	GLContext context;
	if(!context.create("Shapes", VideoMode(windowWidth, windowHeight, 24, 8, 8, false)))
		crash("Failed to create context");

	Renderer renderer;
	renderer.init();

	ShaderProgram 
		defaultShader,
		fboShader;

	if(!defaultShader.loadFromFile("data/shaders/simple.vert", "data/shaders/simple.frag") ||
		!fboShader.loadFromFile("data/shaders/fbopass.vert", "data/shaders/fbopass.frag"))
		crash("Failed to load resources");

	defaultShader.linkAndCheckStatus();
	fboShader.linkAndCheckStatus();

	// Whatever-array-object
	VertexArray vao;
	vao.create();
	vao.bind();

	Mesh cubeMesh = Mesh::getUnitColoredCube();
	MeshBuffer cubeBuffer;
	cubeBuffer.create(cubeMesh);

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

	renderer.enableDepthTest(GL_LEQUAL);
	//renderer.enableCulling(GL_BACK, GL_CCW);
	renderer.setClearDepth(1.0);
	renderer.setClearColor(Color(0.55f, 0.45f, 0.45f, 1.0f));

	mat4 perspectiveMatrix = glm::perspective(45.0f, 640.0f / 480.0f, 0.05f, 50.0f);

	Timer timer;
	timer.start();
	double renderTime = 0.0;

	while(glfwGetWindowParam(GLFW_OPENED) == GL_TRUE)
	{
		timer.step();
		float time = timer.getElapsedTime();

		double renderStart = timer.getElapsedTime();
		renderer.clearColorAndDepth();

		MatrixStack viewMatrix;
		MatrixStack modelMatrix;
		viewMatrix.push();
		viewMatrix.translate(0.0f, 0.0f, -3.0f);

		defaultShader.begin();
		defaultShader.setUniform("projection", perspectiveMatrix);
		defaultShader.setUniform("view", viewMatrix.top());

		modelMatrix.push();
		modelMatrix.rotateY(time);
		modelMatrix.rotateX(0.3f);
		defaultShader.setUniform("model", modelMatrix.top());
		cubeBuffer.draw(GL_TRIANGLES);
		modelMatrix.pop();
		defaultShader.end();
		viewMatrix.pop();

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

	defaultShader.dispose();
	fboShader.dispose();
	cubeBuffer.dispose();
	vao.dispose();
	exit(EXIT_SUCCESS);
}