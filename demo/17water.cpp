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
#include "noise.h"

int main()
{
	const int windowWidth = 640;
	const int windowHeight = 480;
	GLContext context;
	if(!context.create("Terrain", VideoMode(windowWidth, windowHeight, 24, 8, 8, false)))
		shutdown("Failed to create context");

	Renderer renderer;
	renderer.init();

	ShaderProgram 
		simpleShader,
		waterShader;

	if(!simpleShader.loadFromFile("data/shaders/simple.vert", "data/shaders/simple.frag") ||
	   !waterShader.loadFromFile("data/shaders/water.vert", "data/shaders/water.frag"))
		shutdown("Failed to load shaders");

	if(!simpleShader.linkAndCheckStatus() ||
	   !waterShader.linkAndCheckStatus())
		shutdown("Failed to link shaders");

	Mesh cubeMesh = Mesh::genUnitColoredCube();
	MeshBuffer cubeBuffer(cubeMesh);
	Model cube(cubeBuffer);

	Mesh waterMesh = Mesh::genUnitColoredPlane(Color(0.57f, 0.63f, 0.98f));
	MeshBuffer waterBuffer(waterMesh);
	Model water(waterBuffer);

	Mesh quadMesh;
	quadMesh.addPosition(-1.0f, -1.0f, 0.0f);
	quadMesh.addPosition(+1.0f, -1.0f, 0.0f);
	quadMesh.addPosition(+1.0f, +1.0f, 0.0f);
	quadMesh.addPosition(-1.0f, +1.0f, 0.0f);
	quadMesh.addTexel(0.0f, 0.0f);
	quadMesh.addTexel(1.0f, 0.0f);
	quadMesh.addTexel(1.0f, 1.0f);
	quadMesh.addTexel(0.0f, 1.0f);
	quadMesh.addTriangle(0, 1, 2);
	quadMesh.addTriangle(2, 3, 0);
	MeshBuffer quadBuffer(quadMesh);
	Model quad(quadBuffer);

	VertexArray vao;
	vao.create();
	vao.bind();

	renderer.setClearColor(0.55f, 0.45f, 0.45f, 1.0f);
	renderer.setClearDepth(1.0);

	mat4 perspectiveMatrix = glm::perspective(45.0f, windowWidth / float(windowHeight), 0.05f, 50.0f);

	RenderTexture rt(windowWidth, windowHeight);

	Timer timer;
	timer.start();
	double renderTime = 0.0;

	float yAxisRotation = 0.0f;
	float xAxisRotation = -0.78f;
	bool reloading = false;

	while(context.isOpen())
	{
		timer.step();
		float time = timer.getElapsedTime();

		if(glfwGetKey(GLFW_KEY_LEFT))
			yAxisRotation -= 0.78f * timer.getDelta();
		if(glfwGetKey(GLFW_KEY_RIGHT))
			yAxisRotation += 0.78f * timer.getDelta();
		if(glfwGetKey(GLFW_KEY_UP))
			xAxisRotation += 0.78f * timer.getDelta();
		if(glfwGetKey(GLFW_KEY_DOWN))
			xAxisRotation -= 0.78f * timer.getDelta();

		if(glfwGetKey('R') && !reloading)
		{
			reloading = true;

			if(!simpleShader.loadFromFile("data/shaders/simple.vert", "data/shaders/simple.frag") ||
			   !waterShader.loadFromFile("data/shaders/water.vert", "data/shaders/water.frag"))
				shutdown("Failed to load shaders");

			if(!simpleShader.linkAndCheckStatus() ||
			   !waterShader.linkAndCheckStatus())
				shutdown("Failed to link shaders");
		}
		else if(!glfwGetKey('R') && reloading)
		{
			reloading = false;
		}

		double renderStart = timer.getElapsedTime();

		MatrixStack viewMatrix;
		viewMatrix.push();
		viewMatrix.translate(0.0f, 0.0f, -3.0f);
		viewMatrix.rotateX(xAxisRotation);
		viewMatrix.rotateY(yAxisRotation);

		// Render unmirrored scene
		simpleShader.begin();
		renderer.clearColorAndDepth();
		renderer.setCullState(CullStates::CullNone);
		renderer.setBlendState(BlendStates::AlphaBlend);
		renderer.setDepthTestState(DepthTestStates::LessThanOrEqual);
		simpleShader.setUniform("projection", perspectiveMatrix);
		simpleShader.setUniform("view", viewMatrix.top());
		cube.pushTransform();
		cube.translate(0.0f, 0.5f, 0.0f);
		cube.scale(0.5f);
		cube.draw(GL_TRIANGLES);

		// Draw mirrored scene to a rendertarget
		rt.begin();
		renderer.clearColorAndDepth();
		viewMatrix.push();
		viewMatrix.scale(1.0f, -1.0f, 1.0f);
		simpleShader.setUniform("view", viewMatrix.top());
		cube.draw(GL_TRIANGLES);
		cube.popTransform();
		viewMatrix.pop();
		rt.end();

		// Enable stencil testing and mask out a section containing the water mesh
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 1, 0xFF); // Set any stencil to 1
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF); // Write to stencil buffer
		glDepthMask(GL_FALSE); // Don't write to depth buffer
		glClear(GL_STENCIL_BUFFER_BIT); // Clear stencil buffer (0 by default)

		// Draw water mesh
		water.pushTransform();
		water.scale(3.0f);
		water.draw(GL_TRIANGLES);
		water.popTransform();
		simpleShader.end();

		// Draw previous rendertarget as a quad masked into the water plane
		glStencilFunc(GL_EQUAL, 1, 0xFF); // Pass test if stencil value is 1
		glStencilMask(0x00); // Don't write anything to stencil buffer
		glDepthMask(GL_TRUE);

		waterShader.begin();
		rt.bindTexture();
		waterShader.setUniform("time", time);
		waterShader.setUniform("tex", 0);
		waterShader.setUniform("light0Position", vec3(0.0f, 1.0f, 0.0f));
		waterShader.setUniform("light0Color", vec3(1.0f, 0.8f, 0.5f));
		waterShader.setUniform("ambient", vec3(67.0f/255.0f, 66.0f/255.0f, 63.0f/255.0f));
		quad.draw(GL_TRIANGLES);
		rt.unbindTexture();
		waterShader.end();

		glDisable(GL_STENCIL_TEST);

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

	simpleShader.dispose();
	waterShader.dispose();
	vao.dispose();
	return 0;
}