#include <gl/program.h>
#include <iostream>

Program::Program() : program(0)
{ }

void Program::dispose()
{
	unuse();
	glDeleteProgram(program); // OpenGL will silently ignore a value of 0 for program
	program = 0;
}

void Program::use() const
{
	glUseProgram(program);
}

void Program::unuse() const
{
	glUseProgram(0);
}

void Program::create()
{
	dispose();
	program = glCreateProgram();
}

bool Program::linkAndCheckStatus(const std::vector<Shader> &shaders)
{
	if (shaders.size() == 0)
	{
		APP_LOG << "No shaders to be linked\n";
		return false;
	}

	for(unsigned int i = 0; i < shaders.size(); ++i)
		glAttachShader(program, shaders[i].getHandle());

	glLinkProgram(program);

	// Shaders can be detached after the program has been linked
	for(unsigned int i = 0; i < shaders.size(); ++i)
		glDetachShader(program, shaders[i].getHandle());

	// Check for and print errors
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if(status == GL_FALSE)
	{
		GLint length;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
		std::vector<GLchar> log(length);
		glGetProgramInfoLog(program, length, NULL, &log[0]);
		APP_LOG << "Linker failure:\n" << &log[0] << '\n';
		return false;
	}

	return true;	
}

bool Program::linkAndCheckStatus(const Shader &vertexShader, const Shader &fragmentShader)
{
	std::vector<Shader> shaders;
	shaders.push_back(vertexShader);
	shaders.push_back(fragmentShader);
	return linkAndCheckStatus(shaders);
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
void Program::uniform(GLint location, GLdouble d) const { glUniform1f(location, float(d)); }
void Program::uniform(GLint location, GLfloat f) const { glUniform1f(location, f); }
void Program::uniform(GLint location, GLint i) const { glUniform1i(location, i); }