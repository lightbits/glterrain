#include <gl/shaderprogram.h>
#include <graphics/renderer.h>
#include <common/utils.h>

ShaderProgram::ShaderProgram() : 
	m_program(),
	m_shaders(),
	m_uniform_locs(),
	m_attrib_locs() 
{ }

bool ShaderProgram::loadFromSource(const string *sources, GLenum *types, int count)
{
	m_shaders.clear();
	m_uniform_locs.clear();
	m_attrib_locs.clear();
	for (int i = 0; i < count; ++i)
	{
		Shader shader;
		if (!shader.loadFromSource(sources[i], types[i]))
			return false;
		m_shaders.push_back(shader);
	}
	m_program.create();
	return true;
}

bool ShaderProgram::loadFromFile(const string *paths, GLenum *types, int count)
{
	m_shaders.clear();
	m_uniform_locs.clear();
	m_attrib_locs.clear();
	for (int i = 0; i < count; ++i)
	{
		Shader shader;
		if (!shader.loadFromFile(paths[i], types[i]))
			return false;
		m_shaders.push_back(shader);
	}
	m_program.create();
	return true;
}

bool ShaderProgram::loadFromSource(const string &vertSrc, const string &fragSrc)
{
	string sources[] = { vertSrc, fragSrc };
	GLenum types[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
	return loadFromSource(sources, types, 2);
}

bool ShaderProgram::loadFromSource(const string &vertSrc, const string &fragSrc, const string &geomSrc)
{
	string sources[] = { vertSrc, fragSrc, geomSrc };
	GLenum types[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER };
	return loadFromSource(sources, types, 3);
}

bool ShaderProgram::loadFromFile(const string &vertPath, const string &fragPath)
{
	string paths[] = { vertPath, fragPath };
	GLenum types[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
	return loadFromFile(paths, types, 2);
}

bool ShaderProgram::loadFromFile(const string &vertPath, const string &fragPath, const string &geomPath)
{
	string sources[] = { vertPath, fragPath, geomPath };
	GLenum types[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER };
	return loadFromFile(sources, types, 3);
}

bool ShaderProgram::loadFromFile(const string &baseName)
{
	string vs_path = baseName + ".vs";
	string fs_path = baseName + ".fs";
	return loadFromFile(vs_path, fs_path);
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
	return m_program.linkAndCheckStatus(m_shaders);
}

void ShaderProgram::dispose()
{
	m_uniform_locs.clear();
	m_attrib_locs.clear();
	for (int i = 0; i < m_shaders.size(); ++i)
		m_shaders[i].dispose();
	m_program.dispose();
}

void ShaderProgram::begin()
{
	m_program.use();
	//Renderer *r = getActiveRenderer();
	//if(r) r->beginCustomShader(*this);
}

void ShaderProgram::end()
{
	m_program.unuse();
	//Renderer *r = getActiveRenderer();
	//if(r) r->endCustomShader();
}

void ShaderProgram::bindFragDataLocation(const string &name, GLuint colorNumber)
{
	glBindFragDataLocation(m_program.getHandle(), colorNumber, name.c_str());
}

void ShaderProgram::bindAttribute(GLuint location, const string &name)
{
	glBindAttribLocation(m_program.getHandle(), location, name.c_str());
}

GLint ShaderProgram::getUniformLocation(const string &name)
{
	std::unordered_map<string, GLint>::iterator it = m_uniform_locs.find(name);
	if(it != m_uniform_locs.end())
	{
		return it->second;
	}
	else
	{
		GLint location = glGetUniformLocation(m_program.getHandle(), name.c_str());
		if(location < 0)
			throw std::runtime_error("Invalid shader uniform [" + name + "] (not used or bad name)");
		m_uniform_locs[name] = location;
		return location;
	}
}

GLint ShaderProgram::getAttributeLocation(const string &name)
{
	std::unordered_map<string, GLint>::iterator it = m_attrib_locs.find(name);
	if(it != m_attrib_locs.end())
	{
		return it->second;
	}
	else
	{
		GLint location = glGetAttribLocation(m_program.getHandle(), name.c_str());
		if(location < 0)
			throw std::runtime_error("Invalid shader attribute [" + name + "] (not used or bad name)");
		m_attrib_locs[name] = location;
		return location;
	}
}

GLuint ShaderProgram::getHandle() const
{
	return m_program.getHandle();
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

void ShaderProgram::setUniform(const string &name, const mat4 &mat) { m_program.uniform(getUniformLocation(name), mat); }
void ShaderProgram::setUniform(const string &name, const mat3 &mat) { m_program.uniform(getUniformLocation(name), mat); }
void ShaderProgram::setUniform(const string &name, const mat2 &mat) { m_program.uniform(getUniformLocation(name), mat); }
void ShaderProgram::setUniform(const string &name, const vec4 &vec) { m_program.uniform(getUniformLocation(name), vec); }
void ShaderProgram::setUniform(const string &name, const vec3 &vec) { m_program.uniform(getUniformLocation(name), vec); }
void ShaderProgram::setUniform(const string &name, const vec2 &vec) { m_program.uniform(getUniformLocation(name), vec); }
void ShaderProgram::setUniform(const string &name, GLdouble d)      { m_program.uniform(getUniformLocation(name), d); }
void ShaderProgram::setUniform(const string &name, GLfloat f)       { m_program.uniform(getUniformLocation(name), f); }
void ShaderProgram::setUniform(const string &name, GLint i)         { m_program.uniform(getUniformLocation(name), i); }