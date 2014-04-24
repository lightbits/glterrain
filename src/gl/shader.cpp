#include <gl/shader.h>
#include <common/utils.h> // for readFile
#include <iostream> // for cerr

Shader::Shader(GLenum shaderType_) : 
	shader(0), shaderType(shaderType_) 
{ }

void Shader::dispose()
{
	glDeleteShader(shader);
}

void Shader::create()
{
	dispose();
	shader = glCreateShader(shaderType);
}

bool Shader::loadFromSource(const std::string &src)
{
	create();
	return compileAndCheckStatus(src);
}

bool Shader::loadFromFile(const std::string &filename)
{
	std::string src;
	if(!readFile(filename, src))
		return false;

	return loadFromSource(src);
}

bool Shader::compileAndCheckStatus(const std::string &src)
{
	const char *srcCStr = src.c_str();
	glShaderSource(shader, 1, &srcCStr, NULL);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if(status == GL_FALSE)
	{
		GLint length;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		std::vector<GLchar> log(length);
		glGetShaderInfoLog(shader, length, NULL, &log[0]);
		std::cerr<<"Compile failure: "<<&log[0]<<std::endl;
		return false;
	}

	return true;
}

GLuint Shader::getHandle() const
{
	return shader;
}