#include <gl/shaderprogram.h>
#include <graphics/renderer.h>

ShaderProgram::ShaderProgram() : program(), 
	fragmentShader(GL_FRAGMENT_SHADER), 
	vertexShader(GL_VERTEX_SHADER), 
	uniformLocations(),
	attribLocations() { }

bool ShaderProgram::loadFromSource(const string &vertSrc, const string &fragSrc)
{
	uniformLocations.clear();
	attribLocations.clear();
	if(!vertexShader.loadFromSource(vertSrc) || 
	   !fragmentShader.loadFromSource(fragSrc))
		return false;
	program.create();
	return true;
}

bool ShaderProgram::loadFromFile(const string &vertName, const string &fragName)
{
	uniformLocations.clear();
	attribLocations.clear();
	if(!vertexShader.loadFromFile(vertName) || 
	   !fragmentShader.loadFromFile(fragName))
		return false;
	program.create();
	return true;
}

bool ShaderProgram::loadFromFile(const string &baseName)
{
	return loadFromFile(baseName + ".vs", baseName + ".fs");
}

bool ShaderProgram::loadAndLinkFromFile(const string &baseName)
{
	if (!loadFromFile(baseName))
		return false;
	if (!linkAndCheckStatus())
		return false;
	return true;
}

bool ShaderProgram::linkAndCheckStatus()
{
	return program.linkAndCheckStatus(vertexShader, fragmentShader);
}

void ShaderProgram::dispose()
{
	// if(inUse) throw std::exception...
	uniformLocations.clear();
	attribLocations.clear();
	fragmentShader.dispose();
	vertexShader.dispose();
	program.dispose();
}

void ShaderProgram::begin()
{
	program.use();
	//Renderer *r = getActiveRenderer();
	//if(r) r->beginCustomShader(*this);
}

void ShaderProgram::end()
{
	program.unuse();
	//Renderer *r = getActiveRenderer();
	//if(r) r->endCustomShader();
}

void ShaderProgram::bindFragDataLocation(const string &name, GLuint colorNumber)
{
	glBindFragDataLocation(program.getHandle(), colorNumber, name.c_str());
}

void ShaderProgram::bindAttribute(GLuint location, const string &name)
{
	glBindAttribLocation(program.getHandle(), location, name.c_str());
}

GLint ShaderProgram::getUniformLocation(const string &name)
{
	std::unordered_map<string, GLint>::iterator it = uniformLocations.find(name);
	if(it != uniformLocations.end())
	{
		return it->second;
	}
	else
	{
		GLint location = glGetUniformLocation(program.getHandle(), name.c_str());
		if(location < 0)
			throw std::runtime_error("Invalid shader uniform [" + name + "] (not used or bad name)");
		uniformLocations[name] = location;
		return location;
	}
}

GLint ShaderProgram::getAttributeLocation(const string &name)
{
	std::unordered_map<string, GLint>::iterator it = attribLocations.find(name);
	if(it != attribLocations.end())
	{
		return it->second;
	}
	else
	{
		GLint location = glGetAttribLocation(program.getHandle(), name.c_str());
		if(location < 0)
			throw std::runtime_error("Invalid shader attribute [" + name + "] (not used or bad name)");
		attribLocations[name] = location;
		return location;
	}
}

void ShaderProgram::setAttributefv(const string &name, 
								   GLsizei numComponents, 
								   GLsizei stride, 
								   GLsizei offset)
{
	setAttributefv(getAttributeLocation(name), numComponents, stride, offset);
}

void ShaderProgram::setAttributefv(GLint location, 
								   GLsizei numComponents, 
								   GLsizei stride, 
								   GLsizei offset)
{
	// Debug
	//if(location < 0)
	//	throw std::runtime_error("Invalid shader attribute");

	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 
	                      numComponents, 
	                      GL_FLOAT, 
	                      GL_FALSE, 
	                      stride * sizeof(GLfloat), 
	                      reinterpret_cast<void*>(offset * sizeof(GLfloat)));
}

void ShaderProgram::setUniform(const string &name, const mat4 &mat) { program.uniform(getUniformLocation(name), mat); }
void ShaderProgram::setUniform(const string &name, const mat3 &mat) { program.uniform(getUniformLocation(name), mat); }
void ShaderProgram::setUniform(const string &name, const mat2 &mat) { program.uniform(getUniformLocation(name), mat); }
void ShaderProgram::setUniform(const string &name, const vec4 &vec) { program.uniform(getUniformLocation(name), vec); }
void ShaderProgram::setUniform(const string &name, const vec3 &vec) { program.uniform(getUniformLocation(name), vec); }
void ShaderProgram::setUniform(const string &name, const vec2 &vec) { program.uniform(getUniformLocation(name), vec); }
void ShaderProgram::setUniform(const string &name, GLdouble d)      { program.uniform(getUniformLocation(name), d); }
void ShaderProgram::setUniform(const string &name, GLfloat f)       { program.uniform(getUniformLocation(name), f); }
void ShaderProgram::setUniform(const string &name, GLint i)         { program.uniform(getUniformLocation(name), i); }