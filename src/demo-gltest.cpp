#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // for glm::ortho and glm::perspective
#include <glm/gtc/type_ptr.hpp> // for value_ptr(matrix)

#include <opengl.h>
#include <color.h>
#include <helpers.h>
#include <texture.h>
#include <timer.h>

using namespace glm;

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

struct TriMesh
{
	std::vector<vec3> positions;
	std::vector<vec3> normals;
	std::vector<vec4> colors;
	std::vector<vec2> texCoords;
	std::vector<GLushort> indices;

	void position(float x, float y, float z)
	{ positions.push_back(vec3(x, y, z)); }

	void normal(float x, float y, float z)
	{ normals.push_back(vec3(x, y, z)); }

	void color(float r, float g, float b, float a)
	{ colors.push_back(vec4(r, g, b, a)); }

	void texCoord(float u, float v)
	{ texCoords.push_back(vec2(u, v)); }

	GLushort getLastVertexIndex() const
	{ return positions.size() - 1; }

	void addTriangle(GLushort i0, GLushort i1, GLushort i2)
	{ indices.push_back(i0); indices.push_back(i1); indices.push_back(i2); }
};

/*
Upload the mesh data into a GPU memory buffer, using a non-interlaced vertex format,
position0-position1-position2-...
normal0-normal1-normal2-...
color0-color1-color2-...
uv0-uv1-uv2-...
*/
void bufferMesh(const TriMesh &mesh, GLuint buffer)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	// Allocate the whole buffer
	glBufferData(GL_ARRAY_BUFFER, 
		mesh.positions.size() * sizeof(vec3) + 
		mesh.normals.size() * sizeof(vec3) + 
		mesh.colors.size() * sizeof(vec4) +
		mesh.texCoords.size() * sizeof(vec2),
		NULL,
		GL_STATIC_DRAW);

	// Upload the vertex data arrays seperately

	GLintptr offset = 0;
	glBufferSubData(GL_ARRAY_BUFFER, offset, mesh.positions.size() * sizeof(vec3), &mesh.positions[0]);
	offset += mesh.positions.size() * sizeof(vec3);

	if(mesh.normals.size() > 0)
	{
		glBufferSubData(GL_ARRAY_BUFFER, offset, mesh.normals.size() * sizeof(vec3), &mesh.normals[0]);
		offset += mesh.normals.size() * sizeof(vec3);
	}

	if(mesh.colors.size() > 0)
	{
		glBufferSubData(GL_ARRAY_BUFFER, offset, mesh.colors.size() * sizeof(vec4), &mesh.colors[0]);
		offset += mesh.colors.size() * sizeof(vec4);
	}

	if(mesh.texCoords.size() > 0)
		glBufferSubData(GL_ARRAY_BUFFER, offset, mesh.texCoords.size() * sizeof(vec2), &mesh.texCoords[0]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void renderMesh(const TriMesh &mesh, GLuint buffer)
{
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_SHORT, &mesh.indices[0]);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool loadMeshFromObj(const std::string &filename, TriMesh &mesh)
{
	return true;
}

TriMesh genCubeMesh(float hsize)
{
	TriMesh mesh;

	// Front face
	mesh.position(-hsize, -hsize, hsize); mesh.texCoord(0.0f, 0.0f); // Bottom left
	mesh.position( hsize, -hsize, hsize); mesh.texCoord(1.0f, 0.0f); // Bottom right
	mesh.position( hsize,  hsize, hsize); mesh.texCoord(1.0f, 1.0f); // Top right
	mesh.position(-hsize,  hsize, hsize); mesh.texCoord(0.0f, 1.0f); // Top left
	mesh.addTriangle(0, 1, 2);
	mesh.addTriangle(2, 3, 0);

	// Back face
	mesh.position(-hsize, -hsize, -hsize); mesh.texCoord(0.0f, 0.0f); // Bottom left
	mesh.position( hsize, -hsize, -hsize); mesh.texCoord(1.0f, 0.0f); // Bottom right
	mesh.position( hsize,  hsize, -hsize); mesh.texCoord(1.0f, 1.0f); // Top right
	mesh.position(-hsize,  hsize, -hsize); mesh.texCoord(0.0f, 1.0f); // Top left
	mesh.addTriangle(4, 5, 6);
	mesh.addTriangle(6, 7, 4);

	return mesh;
}

int main()
{
	if(!gl::createContext("Terrain", 300, 100, 640, 480, 24, 8, 0, false))
		shutdown("Failed to create context");

	std::string vertexSrc, fragSrc;
	if(!readFile("data/shaders/default.vert", vertexSrc) ||
		!readFile("data/shaders/default.frag", fragSrc))
		return EXIT_FAILURE;

	GLuint vertexShader = gl::createShader(GL_VERTEX_SHADER, vertexSrc.c_str());
	GLuint fragmentShader = gl::createShader(GL_FRAGMENT_SHADER, fragSrc.c_str());
	GLuint shaderList[] = {vertexShader, fragmentShader};
	GLuint program = gl::createProgram(shaderList, 2);

	GLint positionAttribLocation	= glGetAttribLocation(program, "position");
	GLint colorAttribLocation		= glGetAttribLocation(program, "color");
	GLint texCoordAttribLocation	= glGetAttribLocation(program, "texel");
	GLint normalAttribLocation		= glGetAttribLocation(program, "normal");

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLfloat vertices[] = {
		-0.5f, -0.5f, -1.0f, 1.0f, 0.3f, 0.3f, 1.0f,
		 0.5f, -0.5f, -1.0f, 1.0f, 0.3f, 0.3f, 1.0f,
		 0.5f,  0.5f, -1.0f, 1.0f, 0.3f, 0.3f, 1.0f,
		-0.5f,  0.5f, -1.0f, 1.0f, 0.3f, 0.3f, 1.0f
	};

	GLushort indices[] = {0, 1, 2, 2, 3, 0};

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(positionAttribLocation);
	glVertexAttribPointer(positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);

	glEnableVertexAttribArray(colorAttribLocation);
	glVertexAttribPointer(colorAttribLocation, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glDisableVertexAttribArray(positionAttribLocation);
	glDisableVertexAttribArray(colorAttribLocation);

	GLint projectionUniform = glGetUniformLocation(program, "projection");
	GLint viewUniform		= glGetUniformLocation(program, "view");
	GLint modelUniform		= glGetUniformLocation(program, "model");
	GLint texBlendUniform	= glGetUniformLocation(program, "texBlend");

	Timer timer;
	timer.start();
	while(glfwGetWindowParam(GLFW_OPENED) == GL_TRUE)
	{
		glClearColor(0.33f, 0.33f, 0.33f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program);
		glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, value_ptr(mat4(1.0f)));
		glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, value_ptr(mat4(1.0f)));
		glUniform1f(glGetUniformLocation(program, "texBlend"), 0.0f);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindVertexArray(vao);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);

		glfwSwapBuffers();
		glfwSleep(0.013);
	}

	shutdown();
}