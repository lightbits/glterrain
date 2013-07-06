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
#include <graphics/rendermodel.h>
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

ShaderLayout getShaderLayout(const Program &program)
{
	ShaderLayout layout;
	layout.positionAttribIndex	= program.getAttribLocation("position");
	layout.colorAttribIndex		= program.getAttribLocation("color");
	layout.texelAttribIndex		= program.getAttribLocation("texel");
	layout.normalAttribIndex	= program.getAttribLocation("normal");

	layout.projectionUniform	= program.getUniformLocation("projection");
	layout.viewUniform			= program.getUniformLocation("view");
	layout.modelUniform			= program.getUniformLocation("model");
	layout.texBlendUniform		= program.getUniformLocation("texBlend");
	return layout;
}

int main()
{
	if(!gl::createContext("Terrain", 300, 100, 640, 480, 24, 8, 8, false))
		shutdown("Failed to create context");

	std::string 
		default_vert_src, 
		default_frag_src,
		spritebatch_vert_src, 
		sprite_batch_frag_src;

	if(!readFile("data/shaders/default.vert", default_vert_src) ||
		!readFile("data/shaders/default.frag", default_frag_src) ||
		!readFile("data/shaders/spritebatch.vert", spritebatch_vert_src) ||
		!readFile("data/shaders/spritebatch.frag", sprite_batch_frag_src))
		shutdown("Failed to load resources");

	Texture texture0, texture1;
	if(!texture0.loadFromFile("data/img/tex1.png") ||
		!texture1.loadFromFile("data/img/tex0.png"))
		shutdown("Failed to load resources");

	TriMesh mesh0;
	if(!mesh0.loadFromFile("data/mdl/teapot.obj", false))
		shutdown("Failed to load resources");

	Font font;
	if(!font.loadFromFile("data/fonts/proggytinyttsz_8x12.png"))
		shutdown("Failed to load resources");

	Program program0;
	program0.compile(default_vert_src, default_frag_src);

	ShaderLayout program0Layout = getShaderLayout(program0);

	Program program1;
	program1.compile(spritebatch_vert_src, sprite_batch_frag_src);

	DynamicRenderModel model0;
	model0.create(mesh0, program0Layout);

	SpriteBatch spriteBatch0;
	spriteBatch0.create(program1);
	spriteBatch0.setFont(font);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0, 1.0);
	glClearDepth(1.0);

	// Enable culling (hide back facets)
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	// Enable texturing with alpha blending
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	mat4 perspectiveMatrix = glm::perspective(45.0f, 640.0f / 480.0f, 1.0f, 100.0f);

	Timer timer;
	timer.start();
	double renderTime = 0.0;

	while(glfwGetWindowParam(GLFW_OPENED) == GL_TRUE)
	{
		timer.step();

		double renderStart = timer.getElapsedTime();

		glClearColor(0.33f, 0.33f, 0.33f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		MatrixStack modelMatrix;
		modelMatrix.push();
		modelMatrix.translate(0.0f, 0.0f, -10.0f);
		modelMatrix.scale(0.25f);
		modelMatrix.rotateY(timer.getElapsedTime() * 90.0f);
		modelMatrix.rotateX(timer.getElapsedTime() * 45.0f);

		program0.use();
		program0.uniform(program0Layout.projectionUniform, perspectiveMatrix);
		program0.uniform(program0Layout.viewUniform, mat4(1.0f));
		program0.uniform(program0Layout.modelUniform, modelMatrix.top());
		program0.uniform(program0Layout.texBlendUniform, 1.0f);
		texture0.bind();
		model0.draw();
		program0.unuse();

		modelMatrix.pop();

		Text debugText;
		debugText<<"render: "<<renderTime * 1000<<"ms";

		program1.use();
		spriteBatch0.begin();
		spriteBatch0.drawString(debugText.getString(), 5.0f, 5.0f, Colors::White);
		spriteBatch0.end();
		program1.unuse();

		glfwSwapBuffers();
		renderTime = timer.getElapsedTime() - renderStart;
		if(renderTime < 0.013)
			glfwSleep(0.013 - renderTime);
	}

	program0.dispose();
	program1.dispose();
	model0.dispose();
	spriteBatch0.dispose();
	font.dispose();
	texture0.dispose();
	texture1.dispose();
	shutdown();
}