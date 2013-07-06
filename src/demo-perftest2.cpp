#include <graphics/opengl.h>
#include <graphics/texture.h>
#include <graphics/program.h>
#include <graphics/bufferobject.h>
#include <common/helpers.h>
//#define PROGRAM_UNIFORM_APPROACH
#define BUFFER_APPROACH
using namespace graphics;

int main()
{
	gl::createContext("Perf test", 300, 100, 640, 480, 0, 0, 0, false);

	Texture texture0, texture1;
	if(!texture0.loadFromFile("data/img/texture0.png") ||
		!texture1.loadFromFile("data/img/texture1.png"))
		return -1;

	std::string default_vert_src, default_frag_src;

	if(!readFile("data/shaders/default.vert", default_vert_src) ||
		!readFile("data/shaders/default.frag", default_frag_src))
		return -1;

	Program program;
	program.compile(default_vert_src, default_frag_src);
	GLint positionAttribIndex	= program.getAttribLocation("position");
	GLint colorAttribIndex		= program.getAttribLocation("color");
	GLint texelAttribIndex		= program.getAttribLocation("texel");
	GLint projectionUniform		= program.getUniformLocation("projection");
	GLint modelUniform			= program.getUniformLocation("model");
	GLint viewUniform			= program.getUniformLocation("view");
	GLint texBlendUniform		= program.getUniformLocation("texBlend");

	BufferObject vbo;
	vbo.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW);

#ifdef PROGRAM_UNIFORM_APPROACH
	const GLfloat vertices[] = {
		0.0f, 0.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f
	};

	vbo.bind();
	vbo.bufferData(sizeof(vertices), vertiecs);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);

	texture0.setRepeat(true);
	texture1.setRepeat(false);

	while(glfwGetWindowParam(GLFW_OPENED) == GL_TRUE)
	{
		glClearColor(0.33f, 0.33f, 0.33f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		program.use();
		vbo.bind();
		program.enableVertexAttrib(positionAttribIndex, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), 0);
		program.enableVertexAttrib(colorAttribIndex, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
		program.enableVertexAttrib(texelAttribIndex, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void*)(7 * sizeof(GLfloat)));
				
		program.uniform(projectionUniform, glm::ortho(0.0f, 640.0f, 480.0f, 0.0f, 1.0f, 10.0f));
		program.uniform(modelUniform, glm::mat4(1.0f));
		program.uniform(viewUniform, glm::mat4(1.0f));
		program.uniform(texBlendUniform, 1.0f);

#ifdef PROGRAM_UNIFORM_APPROACH
		/*
		This approach works by uploading a single set of vertex data (unit quad),
		and setting the model transformation uniform for each sprite.
		*/
		for(int i = 0; i < 80; ++i)
		{
			float x = (i % 10) * 64.0f;
			float y = (i / 10) * 64.0f;
			float w  = 64.0f;
			float h = 64.0f;

			glm::mat4 m(
				w, 0.0f, 0.0f, 0.0f,
				0.0f, h, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				x, y, 0.0f, 1.0f
				);
			program.uniform(modelUniform, m);
			if(i % 2 == 0)
				texture0.bind();
			else
				texture1.bind();
			glDrawArrays(GL_QUADS, i * 4, 4);
		}
#endif

#ifdef BUFFER_APPROACH
		/*
		This approach works by uploading a set of vertex data for each sprite, using glBufferData.
		The vertices are already transformed.
		*/
		for(int i = 0; i < 80; ++i)
		{
			float x = (i % 10) * 64.0f + i;
			float y = (i / 10) * 64.0f + i;
			float w  = 64.0f;
			float h = 64.0f;
			const GLfloat vertices[] = {
				x, y, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
				x + w, y, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.5f, 1.0f,
				x + w, y + h, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.5f, 0.0f,
				x, y + h, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f
			};

			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			if(i % 2 == 0)
				texture0.bind();
			else
				texture1.bind();
			glDrawArrays(GL_QUADS, 0, 4);
		}
#endif

		glfwSwapBuffers();
	}
}