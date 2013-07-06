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
using namespace graphics;

GLint positionAttribLocation;
GLint colorAttribLocation;
GLint texCoordAttribLocation;
GLint normalAttribLocation;

GLint projectionUniform;
GLint viewUniform;
GLint modelUniform;
GLint texBlendUniform;

class DynamicRenderModel
{
public:
	DynamicRenderModel() : vbo(), ibo(), fmt(), indexCount(0)
	{
		vbo.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW);
		ibo.create(GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_DRAW);
	}

	void dispose()
	{
		vbo.dispose();
		ibo.dispose();
	}

	void create(const TriMesh &mesh)
	{
		vbo.bind();

		vbo.bufferData(mesh.getByteSize(), NULL);
		GLintptr offset = 0;

		if(mesh.positions.size() > 0)
		{
			fmt.addAttrib(positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, offset);
			vbo.bufferSubData(offset, mesh.getPositionsByteSize(), &mesh.positions[0]);
			offset += mesh.getPositionsByteSize();
		}

		if(mesh.normals.size() > 0)
		{
			fmt.addAttrib(normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, offset);
			vbo.bufferSubData(offset, mesh.getNormalsByteSize(), &mesh.normals[0]);
			offset += mesh.getNormalsByteSize();
		}

		if(mesh.colors.size() > 0)
		{
			fmt.addAttrib(colorAttribLocation, 4, GL_FLOAT, GL_FALSE, 0, offset);
			vbo.bufferSubData(offset, mesh.getColorsByteSize(), &mesh.colors[0]);
			offset += mesh.getColorsByteSize();
		}

		if(mesh.texCoords.size() > 0)
		{
			fmt.addAttrib(texCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, offset);
			vbo.bufferSubData(offset, mesh.getTexCoordsByteSize(), &mesh.texCoords[0]);
		}

		vbo.unbind();

		indexCount = mesh.getIndexCount();
		ibo.bind();
		ibo.bufferData(indexCount * sizeof(GLushort), &mesh.indices[0]);
		ibo.unbind();
	}

	void draw()
	{
		vbo.bind();
		ibo.bind();
		fmt.enable();
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);
		fmt.disable();
		vbo.unbind();
		ibo.unbind();
	}
private:
	BufferObject vbo;
	BufferObject ibo;
	VertexFormat fmt;
	int indexCount;
};

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

vec3 bezierPatchPoints[4][4] = {
	{ vec3(-1.0f, 0.0f, -1.0f),		vec3(-0.25f, 0.0f, -1.0f),	vec3(0.25f, 0.0f, -1.0f),	vec3(1.0f, 0.0f, -1.0f) },
	{ vec3(-1.0f, 0.0f, -0.25f),	vec3(-0.25f, 0.0f, -0.25f), vec3(0.25f, 0.0f, -0.25f),	vec3(1.0f, 0.0f, -0.25f) },
	{ vec3(-1.0f, 0.0f, 0.25f),		vec3(-0.25f, 0.0f, 0.25f),	vec3(0.25f, 0.0f, 0.25f),	vec3(1.0f, 0.0f, 0.25f) },
	{ vec3(-1.0f, 0.0f, 1.0f),		vec3(-0.25f, 0.0f, 1.0f),	vec3(0.25f, 0.0f, 1.0f),	vec3(1.0f, 0.0f, 1.0f) }
};

std::pair<int, int> selectedPoint;

void GLFWCALL onKeyPress(int key, int action)
{
	if(action == GLFW_PRESS)
	{
		if(key == GLFW_KEY_LEFT)
			selectedPoint.first = std::max(0, selectedPoint.first - 1);
		else if(key == GLFW_KEY_RIGHT)
			selectedPoint.first = std::min(3, selectedPoint.first + 1);

		if(key == GLFW_KEY_UP)
			selectedPoint.second = std::max(0, selectedPoint.second - 1);
		else if(key == GLFW_KEY_DOWN)
			selectedPoint.second = std::min(3, selectedPoint.second + 1);
	}
}

int main()
{
	if(!gl::createContext("Terrain", 300, 100, 640, 480, 24, 8, 8, false))
		shutdown("Failed to create context");

	std::string default_vert_src, default_frag_src;
	if(!readFile("data/shaders/default.vert", default_vert_src) ||
		!readFile("data/shaders/default.frag", default_frag_src))
		shutdown("Failed to load resources");

	std::string spritebatch_vert_src, sprite_batch_frag_src;
	if(!readFile("data/shaders/spritebatch.vert", spritebatch_vert_src) ||
		!readFile("data/shaders/spritebatch.frag", sprite_batch_frag_src))
		shutdown("Failed to load resources");

	Texture texture0;
	if(!texture0.loadFromFile("data/img/tex1.png"))
		shutdown("Failed to load resources");

	Texture texture1;
	if(!texture1.loadFromFile("data/img/tex0.png"))
		shutdown("Failed to load resources");

	TriMesh mesh0;
	if(!mesh0.loadFromFile("data/mdl/teapot.obj", false))
		shutdown("Failed to load resources");

	Font font;
	if(!font.loadFromFile("data/fonts/proggytinyttsz_8x12.png"))
		shutdown("Failed to load resources");

	Program program;
	program.compile(default_vert_src.c_str(), default_frag_src.c_str());

	Program spriteProgram;
	spriteProgram.compile(spritebatch_vert_src.c_str(), sprite_batch_frag_src.c_str());

	positionAttribLocation	= program.getAttribLocation("position");
	colorAttribLocation		= program.getAttribLocation("color");
	texCoordAttribLocation	= program.getAttribLocation("texel");
	normalAttribLocation	= program.getAttribLocation("normal");

	projectionUniform		= program.getUniformLocation("projection");
	viewUniform				= program.getUniformLocation("view");
	modelUniform			= program.getUniformLocation("model");
	texBlendUniform			= program.getUniformLocation("texBlend");

	DynamicRenderModel model0;
	model0.create(mesh0);

	DynamicRenderModel model1;
	model1.create(TriMesh::genBezierPatch(bezierPatchPoints, 5, 5));

	DynamicRenderModel model2;
	model2.create(TriMesh::genUnitColoredCube());

	SpriteBatch spriteBatch0;
	spriteBatch0.setFont(font);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0, 1.0);
	glClearDepth(1.0);

	/*glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);*/

	glfwSetKeyCallback(onKeyPress);
	selectedPoint = std::pair<int, int>(0, 0);

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

		/*if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_1))
		{
			int x, y;
			glfwGetMousePos(&x, &y);
			bezierPatchPoints[selectedPoint.second][selectedPoint.first].y = -(y - 240) / 240.0f;

			model1.create(TriMesh::genBezierPatch(bezierPatchPoints, 5, 5));
		}*/

		//program.use();
		//program.uniform(projectionUniform, perspectiveMatrix);

		//MatrixStack modelMatrix;
		//MatrixStack viewMatrix;
		//viewMatrix.push();
		//viewMatrix.translate(0.0f, 0.0f, -10.0f);
		//viewMatrix.rotateX(30.0f);

		//// Draw teapot
		//{
		//	modelMatrix.push();
		//	modelMatrix.translate(0.0f, 2.0f, 0.0f);
		//	modelMatrix.rotateY(timer.getElapsedTime() * 180.0f);
		//	modelMatrix.scale(0.1f);

		//	program.uniform(viewUniform, viewMatrix.top());
		//	program.uniform(modelUniform, modelMatrix.top());
		//	program.uniform(texBlendUniform, 1.0f);

		//	glEnable(GL_TEXTURE_2D);
		//	texture0.use();
		//	model0.draw();
		//	texture0.unuse();
		//	glDisable(GL_TEXTURE_2D);

		//	modelMatrix.pop();
		//}

		//// Draw bezier patch
		//{
		//	modelMatrix.push();
		//	modelMatrix.scale(5.0f);
		//	program.uniform(viewUniform, viewMatrix.top());
		//	program.uniform(modelUniform, modelMatrix.top());
		//	program.uniform(texBlendUniform, 0.0f);

		//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//	model1.draw();
		//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//	modelMatrix.pop();
		//}

		//// Draw selected point
		//{
		//	vec3 p = bezierPatchPoints[selectedPoint.second][selectedPoint.first];

		//	modelMatrix.push();
		//	modelMatrix.scale(5.0f);
		//	modelMatrix.translate(p);
		//	modelMatrix.scale(0.1f);
		//	program.uniform(viewUniform, viewMatrix.top());
		//	program.uniform(modelUniform, modelMatrix.top());
		//	program.uniform(texBlendUniform, 0.0f);

		//	model2.draw();

		//	modelMatrix.pop();
		//}

		//viewMatrix.pop();

		//program.unuse();

		Text debugText;
		debugText<<"render: "<<renderTime * 1000<<"ms";
		spriteBatch0.begin(spriteProgram);
		for(int i = 0; i < 20; ++i)
			spriteBatch0.drawTexture(i % 2 == 0 ? texture0 : texture1, Colors::White,
			(i % 10) * 64.0f, (i / 10) * 64.0f, 64.0f, 64.0f,
			0.0f, 1.0f, 0.0f, 1.0f);
		spriteBatch0.drawString(debugText.getString(), 5.0f, 5.0f, Colors::White);
		spriteBatch0.end();

		glfwSwapBuffers();
		renderTime = timer.getElapsedTime() - renderStart;
		glfwSleep(0.013);
	}

	program.dispose();
	spriteProgram.dispose();
	model0.dispose();
	model1.dispose();
	model2.dispose();
	spriteBatch0.dispose();
	font.dispose();
	texture0.dispose();
	texture1.dispose();
	shutdown();
}