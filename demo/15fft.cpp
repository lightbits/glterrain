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
#include <graphics/trimesh.h>
#include <graphics/color.h>
#include <graphics/mesh.h>
#include <graphics/renderer.h>
#include <graphics/model.h>
#include <app/glcontext.h>

void initWireMesh(Mesh &mesh, int samples)
{
	mesh.clear();
	float size = 1.0f;
	float tilesize = size / float(samples);
	for(int y = 0; y < samples; ++y)
	{
		for(int x = 0; x < samples; ++x)
		{
			float xf0 = x * tilesize - size / 2.0f;
			float xf1 = (x + 1) * tilesize - size / 2.0f;
			float yf0 = y * tilesize - size / 2.0f;
			float yf1 = (y + 1) * tilesize - size / 2.0f;
			int i = mesh.getPositionCount();
			mesh.addPosition(xf0, 0.0f, yf0);
			mesh.addPosition(xf1, 0.0f, yf0);
			mesh.addPosition(xf1, 0.0f, yf1);
			mesh.addPosition(xf0, 0.0f, yf1);
			mesh.addColor(1.0f, 1.0f, 1.0f, 1.0f);
			mesh.addColor(1.0f, 1.0f, 1.0f, 1.0f);
			mesh.addColor(1.0f, 1.0f, 1.0f, 1.0f);
			mesh.addColor(1.0f, 1.0f, 1.0f, 1.0f);
			mesh.addTriangle(i + 0, i + 3, i + 2);
			mesh.addTriangle(i + 2, i + 1, i + 0);
		}
	}
}

void updateWireMesh(Mesh &mesh, int samples)
{
	float time = getActiveContext()->getElapsedTime();
	for(int y = 0; y < samples; ++y)
	{
		for(int x = 0; x < samples; ++x)
		{
			int i = (y * samples + x) * 4;
			mesh.getPosition(i + 0).y = 0.07f * sinf((x * y) * 0.1f + time);
			mesh.getPosition(i + 1).y = 0.07f * sinf(((x + 1) * y) * 0.1f + time);
			mesh.getPosition(i + 2).y = 0.07f * sinf(((x + 1) * (y + 1)) * 0.1f + time);
			mesh.getPosition(i + 3).y = 0.07f * sinf((x * (y + 1)) * 0.1f + time);
		}
	}
}

int main()
{
	GLContext context;
	if(!context.create("Cube", VideoMode(640, 480, 24, 8, 8, false)))
		shutdown("Failed to create context");

	Renderer renderer;
	renderer.init();

	ShaderProgram defaultShader;
	if(!defaultShader.loadFromFile("data/shaders/simple.vert", "data/shaders/simple.frag"))
		shutdown("Failed to load resources");

	defaultShader.linkAndCheckStatus();

	Mesh wireMesh;
	const int wireMeshSamples = 8;
	initWireMesh(wireMesh, wireMeshSamples);
	MeshBuffer wireBuffer(wireMesh);
	Model wire(wireBuffer);

	VertexArray vao;
	vao.create();
	vao.bind();

	renderer.setClearColor(0.55f, 0.45f, 0.45f, 1.0f);
	renderer.setClearDepth(1.0);
	renderer.setCullState(CullStates::CullCounterClockwise);
	renderer.setBlendState(BlendStates::AlphaBlend);
	renderer.setDepthTestState(DepthTestStates::LessThanOrEqual);

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
		MatrixStack modelMatrix;
		viewMatrix.push();
		viewMatrix.translate(0.0f, 0.0f, -3.0f);

		defaultShader.begin();
		defaultShader.setUniform("projection", perspectiveMatrix);
		defaultShader.setUniform("view", viewMatrix.top());

		renderer.setRasterizerState(RasterizerStates::LineBoth);

		wireBuffer.bind();
		updateWireMesh(wireMesh, wireMeshSamples);
		wireBuffer.update(wireMesh);
		wireBuffer.unbind();

		wire.pushTransform();
		wire.translate(0.0f, -0.5f, 0.0f);
		wire.scale(3.0f);
		wire.draw(GL_TRIANGLES);
		wire.popTransform();

		renderer.setRasterizerState(RasterizerStates::Default);
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

	defaultShader.dispose();
	vao.dispose();
	return 0;
}