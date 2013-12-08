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
#include <gl/vertexarray.h>
#include <gl/vertexformat.h>
#include <gl/bufferedmesh.h>
#include <gl/shaderprogram.h>
#include <graphics/spritebatch.h>
#include <graphics/trimesh.h>
#include <graphics/color.h>
#include <graphics/mesh.h>
#include <graphics/renderer.h>
#include <app/glcontext.h>

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

	Mesh cubeMesh = Mesh::getUnitColoredCube();
	MeshBuffer cubeBuffer;
	cubeBuffer.create(cubeMesh);

	VertexArray vao;
	vao.create();
	vao.bind();

	renderer.enableDepthTest(GL_LEQUAL);
	renderer.enableCulling(GL_BACK, GL_CCW);
	renderer.setClearColor(Color(0.55f, 0.45f, 0.45f, 1.0f));
	renderer.setClearDepth(1.0);

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

		modelMatrix.push();
		modelMatrix.rotateX(-0.58f);
		modelMatrix.rotateY(time);
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
	vao.dispose();
	return 0;
}