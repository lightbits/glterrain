#include <iostream>
#include <vector>

#include <common/helpers.h>
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

using namespace graphics;
typedef unsigned short uint16;
typedef unsigned int uint32;

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

// Usual attribute locations
GLint positionAttribLocation;
GLint colorAttribLocation;
GLint texCoordAttribLocation;
GLint normalAttribLocation;

// Usual uniform locations
GLint projectionUniform;
GLint viewUniform;
GLint modelUniform;
GLint texBlendUniform;

class RenderModel
{
public:
	RenderModel() : vbo(), fmt(), mesh(), vao(0)
	{

	}

	RenderModel(const TriMesh &mesh_, bool dynamic) : mesh(mesh_), vao(0)
	{
		vbo.create(GL_ARRAY_BUFFER, (dynamic ? GL_STREAM_DRAW : GL_STATIC_DRAW));
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
			fmt.addAttrib(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, offset);
			vbo.bufferSubData(offset, mesh.getColorsByteSize(), &mesh.colors[0]);
			offset += mesh.getColorsByteSize();
		}

		if(mesh.texCoords.size() > 0)
		{
			fmt.addAttrib(texCoordAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, offset);
			vbo.bufferSubData(offset, mesh.getTexCoordsByteSize(), &mesh.texCoords[0]);
		}

		vbo.unbind();
	}

	void dispose()
	{
		vbo.dispose();
		glDeleteVertexArrays(1, &vao);
	}

	void updateBuffer()
	{
		vbo.bind();
		vbo.bufferData(mesh);
		vbo.unbind();
	}

	void genVertexArray()
	{
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		vbo.bind();
		fmt.enable();
		glBindVertexArray(0);
		fmt.disable();
		vbo.unbind();
	}

	void draw()
	{
		vbo.bind();
		if(vao != 0) glBindVertexArray(vao);
		else fmt.enable(); // Enable vertex attrib arrays / specify vertex attrib format

		glDrawElements(GL_TRIANGLES, mesh.getIndexCount(), GL_UNSIGNED_SHORT, reinterpret_cast<void*>(&mesh.indices[0]));

		if(vao != 0) glBindVertexArray(0);
		else fmt.disable();
		vbo.unbind();
	}
private:
	BufferObject vbo;
	VertexFormat fmt;
	TriMesh mesh;
	GLuint vao;
};

int main()
{
	if(!gl::createContext("Terrain", 300, 100, 640, 480, 24, 8, 8, false))
		shutdown("Failed to create context");

	std::string vertexSrc, fragSrc;
	if(!readFile("data/shaders/default.vert", vertexSrc) ||
		!readFile("data/shaders/default.frag", fragSrc))
		shutdown("Failed to load resources");

	Texture texture0;
	if(!texture0.loadFromFile("data/img/tex1.png"))
		shutdown("Failed to load resources");

	TriMesh mesh0;
	if(!mesh0.loadFromFile("data/mdl/teapot.obj", false))
		shutdown("Failed to load resources");

	Program program;
	program.compile(vertexSrc.c_str(), fragSrc.c_str());

	positionAttribLocation	= program.getAttribLocation("position");
	colorAttribLocation		= program.getAttribLocation("color");
	texCoordAttribLocation	= program.getAttribLocation("texel");
	normalAttribLocation	= program.getAttribLocation("normal");

	projectionUniform		= program.getUniformLocation("projection");
	viewUniform				= program.getUniformLocation("view");
	modelUniform			= program.getUniformLocation("model");
	texBlendUniform			= program.getUniformLocation("texBlend");

	BufferObject mesh0Buffer;
	VertexFormat mesh0Format;
	mesh0Format.addAttrib(positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
	mesh0Format.addAttrib(normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, mesh0.getPositionsByteSize());
	mesh0Format.addAttrib(texCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, mesh0.getPositionsByteSize() + mesh0.getNormalsByteSize());
	mesh0Buffer.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	mesh0Buffer.bufferData(mesh0);

	selectedPoint = std::pair<int, int>(0, 0);

	TriMesh mesh1;
	mesh1.genBezierPatch(bezierPatchPoints, 10, 10);
	BufferObject mesh1Buffer;
	VertexFormat mesh1Format;
	mesh1Format.addAttrib(positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
	mesh1Format.addAttrib(colorAttribLocation, 4, GL_FLOAT, GL_FALSE, 0, mesh1.getPositionsByteSize());
	mesh1Buffer.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW);
	mesh1Buffer.bufferData(mesh1);

	TriMesh mesh2;
	mesh2.genUnitColoredCube();
	BufferObject mesh2Buffer;
	VertexFormat mesh2Format;
	mesh2Format.addAttrib(positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
	mesh2Format.addAttrib(colorAttribLocation, 4, GL_FLOAT, GL_FALSE, 0, mesh2.getPositionsByteSize());
	mesh2Buffer.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW);
	mesh2Buffer.bufferData(mesh2);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	mesh0Buffer.bind();
	mesh0Format.enable();
	mesh0Buffer.unbind();

	glBindVertexArray(0);
	mesh0Format.disable();

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0, 1.0);
	glClearDepth(1.0);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glfwSetKeyCallback(onKeyPress);

	mat4 perspectiveMatrix = glm::perspective(45.0f, 640.0f / 480.0f, 1.0f, 100.0f);

	Timer timer;
	timer.start();
	while(glfwGetWindowParam(GLFW_OPENED) == GL_TRUE)
	{
		timer.step();
		glClearColor(0.33f, 0.33f, 0.33f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_1))
		{
			int x, y;
			glfwGetMousePos(&x, &y);
			bezierPatchPoints[selectedPoint.second][selectedPoint.first].y = -(y - 240) / 240.0f;

			mesh1.genBezierPatch(bezierPatchPoints, 10, 10);
			mesh1Buffer.bufferData(mesh1);
		}

		program.use();
		program.uniform(projectionUniform, perspectiveMatrix);

		MatrixStack modelMatrix;
		MatrixStack viewMatrix;
		viewMatrix.push();
		viewMatrix.translate(0.0f, 0.0f, -10.0f);
		viewMatrix.rotateX(30.0f);

		// Draw teapot
		{
			modelMatrix.push();
			modelMatrix.translate(0.0f, 2.0f, 0.0f);
			modelMatrix.rotateY(timer.getElapsedTime() * 180.0f);
			modelMatrix.scale(0.1f);

			program.uniform(viewUniform, glm::inverse(viewMatrix.top()));
			program.uniform(modelUniform, modelMatrix.top());
			program.uniform(texBlendUniform, 1.0f);

			glEnable(GL_TEXTURE_2D);
			texture0.use();
			glBindVertexArray(vao);
			mesh0Buffer.bind();
			glDrawElements(GL_TRIANGLES, mesh0.getIndexCount(), GL_UNSIGNED_SHORT, reinterpret_cast<void*>(&mesh0.indices[0]));
			mesh0Buffer.unbind();
			glBindVertexArray(0);
			texture0.unuse();
			glDisable(GL_TEXTURE_2D);

			modelMatrix.pop();
		}

		// Draw bezier patch
		{
			modelMatrix.push();
			modelMatrix.scale(5.0f);
			program.uniform(viewUniform, viewMatrix.top());
			program.uniform(modelUniform, modelMatrix.top());
			program.uniform(texBlendUniform, 0.0f);

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			mesh1Buffer.bind();
			mesh1Format.enable();
			glDrawElements(GL_TRIANGLES, mesh1.getIndexCount(), GL_UNSIGNED_SHORT, reinterpret_cast<void*>(&mesh1.indices[0]));
			mesh1Format.disable();
			mesh1Buffer.unbind();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			modelMatrix.pop();
		}

		// Draw selected point
		{
			vec3 p = bezierPatchPoints[selectedPoint.second][selectedPoint.first];

			modelMatrix.push();
			modelMatrix.scale(5.0f);
			modelMatrix.translate(p);
			modelMatrix.scale(0.1f);
			program.uniform(viewUniform, viewMatrix.top());
			program.uniform(modelUniform, modelMatrix.top());
			program.uniform(texBlendUniform, 0.0f);

			mesh2Buffer.bind();
			mesh2Format.enable();
			glDrawElements(GL_TRIANGLES, mesh2.getIndexCount(), GL_UNSIGNED_SHORT, reinterpret_cast<void*>(&mesh2.indices[0]));
			mesh2Format.disable();
			mesh2Buffer.unbind();

			modelMatrix.pop();
		}

		viewMatrix.pop();

		program.unuse();

		glfwSwapBuffers();
		glfwSleep(0.013);
	}

	shutdown();
}