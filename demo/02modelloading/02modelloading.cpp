/* Load a .obj mesh.
A wireframe is drawn by using the singlepass technique described in 
http://cgg-journal.com/2008-2/06/index.html
http://codeflow.org/entries/2012/aug/02/easy-wireframe-display-with-barycentric-coordinates/
*/

#include <iostream>
#include <fstream>
#include <vector>

#include <common/helpers.h>
#include <common/text.h>
#include <common/timer.h>
#include <common/vec.h>
#include <common/matrix.h>
#include <common/matrixstack.h>

#include <gl/bufferobject.h>
#include <gl/vertexarray.h>
#include <gl/vertexformat.h>
#include <gl/bufferedmesh.h>
#include <gl/shaderprogram.h>
#include <graphics/spritebatch.h>
#include <graphics/color.h>
#include <graphics/mesh.h>
#include <graphics/renderer.h>
#include <graphics/model.h>
#include <graphics/rendertexture.h>
#include <app/glcontext.h>

GLContext context;
Renderer renderer;

const int windowWidth = 640;
const int windowHeight = 480;

ShaderProgram defaultShader;
Mesh teapotMesh;

bool initialize()
{
	if(!context.create("Model Loading", VideoMode(windowWidth, windowHeight, 24, 8, 8, false)))
		return false;

	renderer.init();
	return true;
}

bool loadShaders()
{
	if(!defaultShader.loadFromFile("data/shaders/diffuse.vert", "data/shaders/diffuse.frag"))
	   return false;

	if(!defaultShader.linkAndCheckStatus())
	   return false;

	return true;
}

bool loadContent()
{
	if(!loadShaders())
		return false;

	if(!teapotMesh.loadFromFile("data/models/teapot.obj"))
		return false;
	teapotMesh.clearTexels();

	return true;
}

void run()
{
	if(!initialize())
		shutdown("Failed to initialize");

	if(!loadContent())
		shutdown("Failed to load resources");

	MeshBuffer teapotBuffer(teapotMesh);
	Model teapot(teapotBuffer);

	VertexArray vao;
	vao.create();
	vao.bind();

	mat4 perspectiveMatrix = glm::perspective(45.0f, windowWidth / float(windowHeight), 0.05f, 50.0f);
	renderer.setClearColor(0.55f, 0.45f, 0.45f, 1.0f);
	renderer.setClearDepth(1.0);
	renderer.setCullState(CullStates::CullCounterClockwise);
	renderer.setDepthTestState(DepthTestStates::LessThanOrEqual);

	Timer timer;
	timer.start();
	double renderTime = 0.0;

	float xRot = 0.0f;
	float yRot = 0.0f;

	while(context.isOpen())
	{
		timer.step();
		double time = timer.getElapsedTime();
		double dt = timer.getDelta();

		if(glfwGetKey(GLFW_KEY_LEFT))		yRot -= 0.7f * dt;
		else if(glfwGetKey(GLFW_KEY_RIGHT)) yRot += 0.7f * dt;
		if(glfwGetKey(GLFW_KEY_UP))			xRot -= 0.7f * dt;
		else if(glfwGetKey(GLFW_KEY_DOWN))	xRot += 0.7f * dt;

		if(glfwGetKey('R'))
			if(!loadShaders())
				shutdown("Failed to reload shaders");

		double renderStart = timer.getElapsedTime();

		MatrixStack viewMatrix;
		viewMatrix.push();
		viewMatrix.translate(0.0f, 0.0f, -5.0f);

		renderer.clearColorAndDepth();		
		defaultShader.begin();
		defaultShader.setUniform("projection", perspectiveMatrix);
		defaultShader.setUniform("view", viewMatrix.top());
		teapot.pushTransform();
		teapot.scale(0.1f);
		teapot.rotateX(xRot);
		teapot.rotateY(yRot);
		teapot.draw(GL_TRIANGLES);
		teapot.popTransform();
		defaultShader.end();
		viewMatrix.pop();

		context.display();
		renderTime = timer.getElapsedTime() - renderStart;
		if(renderTime < 0.013)
			context.sleep(0.013 - renderTime);

		if(checkGLErrors(std::cerr))
		{
			std::cin.get();
			context.close();
		}
	}

	vao.dispose();
	teapotBuffer.dispose();
	renderer.dispose();
}

int main()
{
	try
	{
		run();
	}
	catch(std::exception &e)
	{
		std::cerr<<"An error occured: "<<e.what()<<std::endl;
		std::cin.get();
	}

	return 0;
}