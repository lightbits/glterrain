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
	if(!gl::createContext("Simple Terrain", 300, 100, 640, 480, 24, 8, 8, false))
		gl::shutdown("Failed to create context");

	Shader 
		defaultVS(GL_VERTEX_SHADER),
		defaultFS(GL_FRAGMENT_SHADER);

	if(!defaultVS.loadFromFile("data/shaders/wireframe.vert") ||
		!defaultFS.loadFromFile("data/shaders/wireframe.frag"))
		gl::shutdown("Failed to load resources");

	Program program0;

	program0.create();
	program0.linkAndCheckStatus(defaultVS, defaultFS);

	ProgramLayout program0Layout;

	program0Layout.setAttrib("position", program0.getAttribLocation("position"));
	program0Layout.setAttrib("normal", program0.getAttribLocation("normal"));
	program0Layout.setUniform("projection", program0.getUniformLocation("projection"));
	program0Layout.setUniform("model", program0.getUniformLocation("model"));
	program0Layout.setUniform("view", program0.getUniformLocation("view"));

	BufferObject terrainVbo;
	BufferObject terrainIbo;
	terrainVbo.create(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
	terrainIbo.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	terrainVbo.bind();
	terrainIbo.bind();
	terrainIbo.bufferData(mesh0.getIndexCount() * sizeof(GLushort), &mesh0.indices[0]);
	terrainVbo.bufferData(
		barycentric.size() * sizeof(vec3) + 
		mesh0.getPositionsByteSize() + 
		mesh0.getNormalsByteSize(), NULL);

	GLintptr offset = 0;
	meshVbo.bufferSubData(offset, mesh0.getPositionsByteSize(), &mesh0.positions[0]);
	offset += mesh0.getPositionsByteSize();
	meshVbo.bufferSubData(offset, mesh0.getNormalsByteSize(), &mesh0.normals[0]);
	offset += mesh0.getNormalsByteSize();
	meshVbo.bufferSubData(offset, barycentric.size() * sizeof(vec3), &barycentric[0]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
		viewMatrix.translate(0.0f, 0.0f, -5.0f);

		program0.use();
		program0.uniform(program0Layout.getUniformLoc("projection"), perspectiveMatrix);
		program0.uniform(program0Layout.getUniformLoc("view"), viewMatrix.top());

		modelMatrix.push();
		modelMatrix.rotateX(-0.58f);
		modelMatrix.rotateY(time);
		modelMatrix.scale(0.12f);
		program0.uniform(program0Layout.getUniformLoc("model"), modelMatrix.top());

		barycentricVBO.bind();
		glEnableVertexAttribArray(program0Layout.getAttribLoc("barycentric"));
		glVertexAttribPointer(program0Layout.getAttribLoc("barycentric"), 3, GL_FLOAT, GL_FALSE, 0, 0);
		mesh0buffer.draw();
		glDisableVertexAttribArray(program0Layout.getAttribLoc("barycentric"));
		barycentricVBO.unbind();
		/*meshVbo.bind();
		meshIbo.bind();
		glEnableVertexAttribArray(program0Layout.getAttribLoc("position"));
		glVertexAttribPointer(program0Layout.getAttribLoc("position"), 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(program0Layout.getAttribLoc("normal"));
		glVertexAttribPointer(program0Layout.getAttribLoc("normal"), 3, GL_FLOAT, GL_FALSE, 0, 
			(const void*)(mesh0.getPositionsByteSize()));
		glEnableVertexAttribArray(program0Layout.getAttribLoc("barycentric"));
		glVertexAttribPointer(program0Layout.getAttribLoc("barycentric"), 3, GL_FLOAT, GL_FALSE, 0, 
			(const void*)(mesh0.getPositionsByteSize() + mesh0.getNormalsByteSize()));

		glDrawElements(GL_TRIANGLES, mesh0.getIndexCount(), GL_UNSIGNED_SHORT, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);*/
		modelMatrix.pop();

		program0.unuse();
		viewMatrix.pop();

		glfwSwapBuffers();
		renderTime = timer.getElapsedTime() - renderStart;
		if(renderTime < 0.005)
			glfwSleep(0.005 - renderTime);

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
	mesh0buffer.dispose();
	program0.dispose();
	glDeleteVertexArrays(1, &vao);
	gl::shutdown();
}