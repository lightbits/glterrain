#include <gl/program.h>
#include <iostream>

void ProgramLayout::setAttrib(const std::string &name, GLint attrib)
{
	attribs[name] = attrib;
}

void ProgramLayout::setUniform(const std::string &name, GLint uniform)
{
	uniforms[name] = uniform;
}

GLint ProgramLayout::getAttribLoc(const std::string &name) const
{
	std::unordered_map<std::string, GLint>::const_iterator i = attribs.find(name);
	return i != attribs.end() ? i->second : -1;
}

GLint ProgramLayout::getUniformLoc(const std::string &name) const
{
	std::unordered_map<std::string, GLint>::const_iterator i = uniforms.find(name);
	return i != uniforms.end() ? i->second : -1;
}

Program::Program() : program(0)
{
	
}

void Program::dispose() const
{
	unuse();
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

void Program::create()
{
	dispose();
	program = glCreateProgram();
}

bool Program::linkAndCheckStatus(const std::vector<Shader> &shaders)
{
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
		std::cerr<<"Linker failure: "<<std::endl<<&log[0]<<std::endl;
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
void Program::uniform(GLint location, GLfloat f) const { glUniform1f(location, f); }
void Program::uniform(GLint location, GLint i) const { glUniform1i(location, i); }