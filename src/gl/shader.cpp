#include <gl/shader.h>
#include <common/utils.h> // for readFile
#include <iostream> // for cerr

Shader::Shader() : 
	m_handle(0)
{ }

void Shader::dispose()
{
	glDeleteShader(m_handle);
}

bool Shader::loadFromSource(const std::string &src, GLenum type)
{
	m_type = type;
	m_handle = glCreateShader(type);
	return compileAndCheckStatus(src);
}

bool Shader::loadFromFile(const std::string &filename, GLenum type)
{
	std::string src;
	if(!readFile(filename, src))
		return false;

	return loadFromSource(src, type);
}

bool Shader::compileAndCheckStatus(const std::string &src)
{
	const char *srcCStr = src.c_str();
	glShaderSource(m_handle, 1, &srcCStr, NULL);
	glCompileShader(m_handle);

	GLint status;
	glGetShaderiv(m_handle, GL_COMPILE_STATUS, &status);
	if(status == GL_FALSE)
	{
		GLint length;
		glGetShaderiv(m_handle, GL_INFO_LOG_LENGTH, &length);
		std::vector<GLchar> log(length);
		glGetShaderInfoLog(m_handle, length, NULL, &log[0]);
		APP_LOG << "Compile failure in:\n" << src << "\n" << &log[0] <<'\n';
		return false;
	}

	return true;
}

GLuint Shader::getHandle() const
{
	return m_handle;
}