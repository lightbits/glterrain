#include <iostream>
#include <fstream>
#include <vector>

#include <common/helpers.h>
#include <common/text.h>
#include <common/timer.h>
#include <common/vec.h>
#include <common/matrix.h>
#include <common/matrixstack.h>

#include <gl/opengl.h>
#include <gl/texture.h>
#include <gl/program.h>
#include <gl/bufferobject.h>
#include <gl/vertexformat.h>
#include <gl/bufferedmesh.h>
#include <graphics/spritebatch.h>
#include <graphics/trimesh.h>
#include <graphics/color.h>
#include <graphics/renderer.h>
#include <app/glcontext.h>

int main()
{
	GLContext context;
	if(!context.create("Skeleton", VideoMode(640, 480, 24, 8, 8, false)))
		return -1;

	Renderer renderer;
	renderer.init();
	renderer.setClearColor(Color(0.55f, 0.45f, 0.45f, 1.0f));

	while(context.isOpen())
	{
		renderer.clearColorBuffer();

		context.display();
		GLenum error = glGetError();
		if(error != GL_NO_ERROR)
		{
			std::cerr<<getErrorMessage(error)<<"...";
			std::cin.get();
			context.close();
		}
	}

	renderer.dispose();
	shutdown();
}