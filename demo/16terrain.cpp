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
#include <app/glcontext.h>
#include "noise.h"

void initTerrainMesh(Mesh &mesh)
{
	mesh.clear();
	float frequency = 1.1f;
	int samples = 64; // Number of interpolation values to take between start and end noise points
	float ds = frequency / float(samples);
	float scale = 3.0f; // World-space width and depth
	float height = 2.0f; // World-space height

	std::vector<float> heights(samples * samples);
	for(int i = 0; i < heights.size(); ++i)
	{
		int xi = i % samples;
		int yi = i / samples;
		float h0 = min(fBm(xi * ds, yi * ds) * height, 0.35f) * exp(-0.0014f * ((xi - 32) * (xi - 32) + (yi - 32) * (yi - 32)));
		heights[i] = h0;
	}

	for(int y = 0; y < samples - 1; ++y)
	{
		for(int x = 0; x < samples - 1; ++x)
		{
			float h00 = heights[y * samples + x];
			float h10 = heights[y * samples + x + 1];
			float h01 = heights[(y + 1) * samples + x];
			float h11 = heights[(y + 1) * samples + x + 1];
			/*float h00 = fBm(x * ds, y * ds);
			float h10 = fBm((x + 1) * ds, y * ds);
			float h01 = fBm(x * ds, (y + 1) * ds);
			float h11 = fBm((x + 1) * ds, (y + 1) * ds);*/

			float xf0 = (x / float(samples)) * scale - scale * 0.5f;
			float xf1 = ((x + 1) / float(samples)) * scale - scale * 0.5f;
			float zf0 = (y / float(samples)) * scale - scale * 0.5f;
			float zf1 = ((y + 1) / float(samples)) * scale - scale * 0.5f;

			vec3 v0(xf0, h00, zf0);
			vec3 v1(xf1, h10, zf0);
			vec3 v2(xf1, h11, zf1);
			vec3 v3(xf0, h01, zf1);

			vec3 n0 = glm::cross(v0 - v1, v2 - v1);

			int i = mesh.getPositionCount();
			mesh.addPosition(xf0, h00, zf0);
			mesh.addPosition(xf1, h10, zf0);
			mesh.addPosition(xf1, h11, zf1);
			mesh.addPosition(xf0, h01, zf1);
			mesh.addNormal(n0);
			mesh.addNormal(n0);
			mesh.addNormal(n0);
			mesh.addNormal(n0);
			mesh.addTriangle(i + 0, i + 3, i + 2);
			mesh.addTriangle(i + 2, i + 1, i + 0);
		}
	}
}

int main()
{
	GLContext context;
	if(!context.create("Terrain", VideoMode(640, 480, 24, 8, 8, false)))
		shutdown("Failed to create context");

	Renderer renderer;
	renderer.init();

	ShaderProgram 
		terrainShader,
		waterShader;

	if(!terrainShader.loadFromFile("data/shaders/terrain.vert", "data/shaders/terrain.frag") ||
	   !waterShader.loadFromFile("data/shaders/water.vert", "data/shaders/water.frag"))
		shutdown("Failed to load shaders");

	if(!terrainShader.linkAndCheckStatus() ||
	   !waterShader.linkAndCheckStatus())
		shutdown("Failed to link shaders");

	Font font0;
	if(!font0.loadFromFile("data/fonts/proggytinyttsz_8x12.png"))
		shutdown("Failed to load fonts");

	SpriteBatch spriteBatch;
	spriteBatch.setFont(font0);

	Mesh terrainMesh;
	initTerrainMesh(terrainMesh);
	MeshBuffer terrainBuffer(terrainMesh);
	Model terrain(terrainBuffer);

	Mesh waterMesh;
	waterMesh.addPosition(-1.5f, 0.0f, -1.5f);
	waterMesh.addPosition(+1.5f, 0.0f, -1.5f);
	waterMesh.addPosition(+1.5f, 0.0f, +1.5f);
	waterMesh.addPosition(-1.5f, 0.0f, +1.5f);
	waterMesh.addColor(0.57f, 0.63f, 0.98f, 0.5f);
	waterMesh.addColor(0.57f, 0.63f, 0.98f, 0.5f);
	waterMesh.addColor(0.57f, 0.63f, 0.98f, 0.5f);
	waterMesh.addColor(0.57f, 0.63f, 0.98f, 0.5f);
	waterMesh.addNormal(0.0f, 1.0f, 0.0f);
	waterMesh.addNormal(0.0f, 1.0f, 0.0f);
	waterMesh.addNormal(0.0f, 1.0f, 0.0f);
	waterMesh.addNormal(0.0f, 1.0f, 0.0f);
	waterMesh.addTriangle(0, 3, 2);
	waterMesh.addTriangle(2, 1, 0);
	MeshBuffer waterBuffer(waterMesh);
	Model water(waterBuffer);

	VertexArray vao;
	vao.create();
	vao.bind();

	renderer.setClearColor(0.0f, 0.0f, 0.1f, 1.0f);
	//renderer.setClearColor(0.55f, 0.45f, 0.45f, 1.0f);
	renderer.setClearDepth(1.0);

	mat4 perspectiveMatrix = glm::perspective(45.0f, 640.0f / 480.0f, 0.05f, 50.0f);

	Timer timer;
	timer.start();
	double renderTime = 0.0;

	float yAxisRotation = 0.0f;
	float xAxisRotation = -0.78f;

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

		double renderStart = timer.getElapsedTime();
		renderer.clearColorAndDepth();

		MatrixStack viewMatrix;
		MatrixStack modelMatrix;
		viewMatrix.push();
		viewMatrix.translate(0.0f, 0.0f, -3.0f);
		viewMatrix.rotateX(xAxisRotation);
		viewMatrix.rotateY(yAxisRotation);

		renderer.setCullState(CullStates::CullCounterClockwise);
		renderer.setBlendState(BlendStates::AlphaBlend);
		renderer.setDepthTestState(DepthTestStates::LessThanOrEqual);

		terrainShader.begin();
		terrainShader.setUniform("projection", perspectiveMatrix);
		terrainShader.setUniform("view", viewMatrix.top());
		terrainShader.setUniform("light0Position", vec3(0.5f, 1.0f, 0.0f));
		terrainShader.setUniform("light0Color", vec3(1.0f, 0.8f, 0.5f));
		terrainShader.setUniform("ambient", vec3(67.0f/255.0f, 66.0f/255.0f, 63.0f/255.0f));		
		terrain.draw(GL_TRIANGLES);
		terrainShader.end();

		waterShader.begin();
		waterShader.setUniform("projection", perspectiveMatrix);
		waterShader.setUniform("view", viewMatrix.top());
		waterShader.setUniform("light0Position", vec3(0.5f, 1.0f, 0.0f));
		waterShader.setUniform("light0Color", vec3(1.0f, 0.8f, 0.5f));
		waterShader.setUniform("ambient", vec3(67.0f/255.0f, 66.0f/255.0f, 63.0f/255.0f));
		water.pushTransform();
		water.translate(0.0f, -0.15f, 0.0f);
		water.draw(GL_TRIANGLES);
		water.popTransform();
		waterShader.end();

		viewMatrix.pop();

		Text debugInfo;
		debugInfo<<"render: "<<int(renderTime * 1000.0)<<"ms";

		spriteBatch.begin();
		spriteBatch.drawString(debugInfo.getString(), vec2(5.0f, 5.0f), Color(0.44f, 0.55f, 0.89f, 1.0f));
		spriteBatch.end();

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

	terrainShader.dispose();
	vao.dispose();
	font0.dispose();
	return 0;
}