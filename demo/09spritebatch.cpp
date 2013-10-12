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
#include <graphics/vertexarray.h>
using namespace graphics;

int main()
{
	const int width = 640;
	const int height = 480;
	if(!gl::createContext("Spritebatch", 300, 100, width, height, 24, 8, 8, false))
		gl::shutdown("Failed to create context");

	Shader 
		normalShadingVS(GL_VERTEX_SHADER),
		normalShadingFS(GL_FRAGMENT_SHADER),
		spritebatchVS(GL_VERTEX_SHADER),
		spritebatchFS(GL_FRAGMENT_SHADER);

	TriMesh mesh0;

	Font font0;

	Texture texture0;

	if(!normalShadingVS.loadFromFile("data/shaders/normalShading.vert") ||
		!normalShadingFS.loadFromFile("data/shaders/normalShading.frag") ||
		!spritebatchVS.loadFromFile("data/shaders/spritebatch.vert") ||
		!spritebatchFS.loadFromFile("data/shaders/spritebatch.frag") ||
		!mesh0.loadFromFile("data/mdl/teapot.obj", false) ||
		!font0.loadFromFile("data/fonts/proggytinyttsz_8x12.png") ||
		!texture0.loadFromFile("data/img/tex1.png"))
		gl::shutdown("Failed to load resources");

	Program 
		program0,
		program1;

	program0.create();
	program0.linkAndCheckStatus(normalShadingVS, normalShadingFS);

	program1.create();
	program1.linkAndCheckStatus(spritebatchVS, spritebatchFS);

	SpriteBatch spritebatch0;
	spritebatch0.create(program1);
	spritebatch0.setFont(font0);

	ProgramLayout program0Layout;

	program0Layout.setAttrib("position",	program0.getAttribLocation("position"));
	program0Layout.setAttrib("normal",		program0.getAttribLocation("normal"));
	program0Layout.setUniform("projection", program0.getUniformLocation("projection"));
	program0Layout.setUniform("model",		program0.getUniformLocation("model"));
	program0Layout.setUniform("view",		program0.getUniformLocation("view"));

	mesh0.texels.clear();
	BufferedMesh mesh0buffer;
	mesh0buffer.create(mesh0, program0Layout);

	// Whatever-array-object
	VertexArray vao;
	vao.create();
	vao.bind();

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0, 1.0);
	glClearDepth(1.0);

	// Enable alpha blending (transparency)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Enable culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	mat4 perspectiveMatrix = glm::perspective(45.0f, width / float(height), 0.05f, 50.0f);

	Timer timer;
	timer.start();
	double renderTime = 0.0;

	int mouseX = 0;
	int mouseY = 0;

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

		// Draw background
		program1.use();
		spritebatch0.begin();
		spritebatch0.drawTexture(texture0, Color(1.0f), 0.0f, 0.0f, 640.0f, 480.0f);
		spritebatch0.end();
		program1.unuse();
		glClear(GL_DEPTH_BUFFER_BIT);

		program0.use();
		program0.uniform(program0Layout.getUniformLoc("projection"), perspectiveMatrix);
		program0.uniform(program0Layout.getUniformLoc("view"), viewMatrix.top());

		modelMatrix.push();
		modelMatrix.rotateX(-0.58f);
		modelMatrix.rotateY(time);
		modelMatrix.scale(0.12f);
		program0.uniform(program0Layout.getUniformLoc("model"), modelMatrix.top());
		mesh0buffer.draw();
		modelMatrix.pop();
		program0.unuse();
		viewMatrix.pop();

		glfwGetMousePos(&mouseX, &mouseY);
		vec2 mselect = vec2(float(mouseX), float(mouseY));

		GLubyte tex[4];
		glReadPixels(mouseX, height - mouseY, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, tex);

		Text info;
		info<<"render: "<<int(renderTime * 1000)<<"ms";
		program1.use();
		spritebatch0.begin();
		spritebatch0.drawString(info.getString(), 5.0f, 5.0f, Color(1.0f));
		spritebatch0.drawQuad(Color(0.0f), mselect.x + 2.0f, mselect.y + 2.0f, 34.0f, 34.0f);
		spritebatch0.drawQuad(Color(tex[0], tex[1], tex[2], tex[3]), mselect.x + 3.0f, mselect.y + 3.0f, 32.0f, 32.0f);
		spritebatch0.end();
		program1.unuse();

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

	normalShadingVS.dispose();
	normalShadingFS.dispose();
	mesh0buffer.dispose();
	program0.dispose();
	program1.dispose();
	vao.dispose();
	gl::shutdown();
}