#include <iostream>
#include <fstream>
#include <vector>

#include <common/helpers.h>
#include <common/text.h>
#include <common/timer.h>
#include <common/vec.h>
#include <common/matrix.h>
#include <common/matrixstack.h>

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

void shutdown(const char *error = "")
{
	if(error != "")
	{
		std::cerr<<error<<std::endl;
		std::cin.get();
	}
	gl::destroyContext();
	exit(error != "" ? EXIT_FAILURE : EXIT_SUCCESS);
}

int main()
{
	if(!gl::createContext("Terrain", 300, 100, 720, 480, 24, 8, 8, false))
		shutdown("Failed to create context");

	std::string 
		terrain_vert_src, 
		terrain_frag_src,
		texture_vert_src,
		texture_frag_src;

	if(!readFile("data/shaders/terrain.vert", terrain_vert_src) ||
		!readFile("data/shaders/terrain.frag", terrain_frag_src) ||
		!readFile("data/shaders/texture.vert", texture_vert_src) ||
		!readFile("data/shaders/texture.frag", texture_frag_src))
		shutdown("Failed to load resources");

	Program program0;
	program0.compile(terrain_vert_src, terrain_frag_src);

	ShaderLayout program0Layout;
	program0Layout.attribs["position"] = program0.getAttribLocation("position");
	program0Layout.attribs["normal"] = program0.getAttribLocation("normal");
	program0Layout.uniforms["projection"] = program0.getUniformLocation("projection");
	program0Layout.uniforms["model"] = program0.getUniformLocation("model");
	program0Layout.uniforms["view"] = program0.getUniformLocation("view");
	program0Layout.uniforms["white"] = program0.getUniformLocation("white");

	Program program1;
	program1.compile(texture_vert_src, texture_frag_src);

	ShaderLayout program1Layout;
	program1Layout.attribs["position"] = program1.getAttribLocation("position");
	program1Layout.attribs["texel"] = program1.getAttribLocation("texel");

	// A quad mesh to display the noisemap texture
	TriMesh quadMesh;
	BufferedMesh quadBuffer;
	quadMesh.addPosition(-0.5f, -0.5f, 0.0f); quadMesh.addTexel(0.0f, 0.0f);
	quadMesh.addPosition(-0.5f,  0.5f, 0.0f); quadMesh.addTexel(0.0f, 1.0f);
	quadMesh.addPosition( 0.5f,  0.5f, 0.0f); quadMesh.addTexel(1.0f, 1.0f);
	quadMesh.addPosition( 0.5f, -0.5f, 0.0f); quadMesh.addTexel(1.0f, 0.0f);
	quadMesh.addTriangle(0, 1, 2);
	quadMesh.addTriangle(2, 3, 0);
	quadBuffer.create(quadMesh, program1Layout);

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

	glFrontFace(GL_CW);
	glEnable(GL_TEXTURE_2D);

	GLuint heightmapTexture;
	generateHeightmapTexture(heightmapTexture, 96, 96, 4.0f);

	TriMesh terrainMesh;
	BufferedMesh terrainBuffer;
	generateTerrainMesh(terrainMesh, 96, 96, 6.0f, 16.0f);
	terrainBuffer.create(terrainMesh, program0Layout);

	mat4 perspectiveMatrix = glm::perspective(45.0f, 720.0f / 480.0f, 0.05f, 50.0f);

	Timer timer;
	timer.start();
	double renderTime = 0.0;

	while(glfwGetWindowParam(GLFW_OPENED) == GL_TRUE)
	{
		timer.step();

		update(timer.getElapsedTime());

		double renderStart = timer.getElapsedTime();
		glClearColor(0.55f, 0.45f, 0.45f, 1.0f);
		//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		MatrixStack viewMatrix;
		viewMatrix.push();
		viewMatrix.translate(0.0f, -0.7f, -4.0f + zoom);
		MatrixStack modelMatrix;

		program0.use();
		program0.uniform(program0Layout.getUniformLoc("projection"), perspectiveMatrix);
		program0.uniform(program0Layout.getUniformLoc("view"), viewMatrix.top());

		// Draw terrain
			modelMatrix.push();
			modelMatrix.rotateX(rotationX * 180.0f / 3.1415f);
			modelMatrix.rotateY(rotationY * 180.0f / 3.1415f);
			program0.uniform(program0Layout.getUniformLoc("model"), modelMatrix.top());
			program0.uniform(program0Layout.getUniformLoc("white"), 0.0f);
			terrainBuffer.draw();

			if(wireframe)
			{
				program0.uniform(program0Layout.getUniformLoc("white"), 1.0f);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				terrainBuffer.draw();
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			
			modelMatrix.pop();

			
		program0.unuse();
		viewMatrix.pop();

		//program1.use();

		//// Draw heightmap texture
		//	program1.uniform(program1Layout.getUniformLoc("tex"), 0);
		//	glActiveTexture(GL_TEXTURE0 + 0);
		//	glBindTexture(GL_TEXTURE_2D, heightmapTexture);
		//	quadBuffer.draw();
		//	glBindTexture(GL_TEXTURE_2D, 0);

		//program1.unuse();

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

	program0.dispose();
	terrainBuffer.dispose();
	shutdown();
}