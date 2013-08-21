#include <graphics/program.h>
#include <iostream>
using namespace graphics;

Program::Program() : program(0), vertexShader(0), fragmentShader(0)
{
	
}

void Program::dispose() const
{
	unuse();
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteProgram(program); // OpenGL will silently ignore a value of 0 for program
}

void Program::use() const
{
	glUseProgram(program);
}

void Program::unuse() const
{
	glUseProgram(0);
}

void Program::compile(const std::string &vertexSrc, const std::string &fragmentSrc)
{
	dispose();

	vertexShader = gl::compileShader(GL_VERTEX_SHADER, 1, vertexSrc.c_str());
	fragmentShader = gl::compileShader(GL_FRAGMENT_SHADER, 1, fragmentSrc.c_str());
	GLuint shaders[] = {fragmentShader, vertexShader};
	program = gl::createProgram(shaders, 2);
}

GLuint Program::getHandle() const
{
	return program;
}

GLint Program::getAttribLocation(const GLchar *name) const
{
	return glGetAttribLocation(program, name);
}

GLint Program::getUniformLocation(const GLchar *name) const
{
	return glGetUniformLocation(program, name);
}

void Program::enableVertexAttrib(const GLchar *name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *offset) const
{
	GLint location = glGetAttribLocation(program, name);
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, size, type, normalized, stride, offset);
}

void Program::enableVertexAttrib(GLint location, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *offset) const
{
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, size, type, normalized, stride, offset);
}

void Program::bindFragDataLocation(GLint colorNumber, const GLchar *name) const
{
	glBindFragDataLocation(program, colorNumber, name);
}

void Program::uniform(GLint location, const mat4 &mat) const { glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat)); }
void Program::uniform(GLint location, const mat3 &mat) const { glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(mat)); }
void Program::uniform(GLint location, const mat2 &mat) const { glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(mat)); }
void Program::uniform(GLint location, const vec4 &vec) const { glUniform4f(location, vec.x, vec.y, vec.z, vec.w); }
void Program::uniform(GLint location, const vec3 &vec) const { glUniform3f(location, vec.x, vec.y, vec.z); }
void Program::uniform(GLint location, const vec2 &vec) const { glUniform2f(location, vec.x, vec.y); }
void Program::uniform(GLint location, GLfloat f) const { glUniform1f(location, f); }
void Program::uniform(GLint location, GLint i) const { glUniform1i(location, i); }

void Program::uniform(const GLchar *name, const mat4 &mat) const { uniform(glGetUniformLocation(program, name), mat); }
void Program::uniform(const GLchar *name, const mat3 &mat) const { uniform(glGetUniformLocation(program, name), mat); }
void Program::uniform(const GLchar *name, const mat2 &mat) const { uniform(glGetUniformLocation(program, name), mat); }
void Program::uniform(const GLchar *name, const vec4 &vec) const { uniform(glGetUniformLocation(program, name), vec); }
void Program::uniform(const GLchar *name, const vec3 &vec) const { uniform(glGetUniformLocation(program, name), vec); }
void Program::uniform(const GLchar *name, const vec2 &vec) const { uniform(glGetUniformLocation(program, name), vec); }
void Program::uniform(const GLchar *name, GLfloat f) const { uniform(glGetUniformLocation(program, name), f); }
void Program::uniform(const GLchar *name, GLint i) const { uniform(glGetUniformLocation(program, name), i); }