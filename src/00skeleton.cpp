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
	if(!gl::createContext("Skeleton", 300, 100, 640, 480, 24, 8, 8, false))
		shutdown("Failed to create context");

	while(glfwGetWindowParam(GLFW_OPENED) == GL_TRUE)
	{
		glClearColor(0.55f, 0.45f, 0.45f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwSwapBuffers();
		GLenum error = glGetError();
		if(error != GL_NO_ERROR)
		{
			std::cerr<<gl::getErrorMessage(error)<<"...";
			std::cin.get();
			glfwCloseWindow();
		}
	}

	shutdown();
}