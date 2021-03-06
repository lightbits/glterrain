#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H
#include <gl/program.h>
#include <gl/shader.h>
#include <common/typedefs.h>
#include <unordered_map>

class ShaderProgram
{
public:
	ShaderProgram();
	ShaderProgram(const Program &program);

	/* Loads shaders from an array of sources with corresponding types  */
	bool loadFromSource(const string *sources, GLenum *types, int count);
	bool loadFromSource(const string &vertSrc, const string &fragSrc);
	bool loadFromSource(const string &vertSrc, const string &fragSrc, const string &geomSrc);

	bool loadFromFile(const string *paths, GLenum *types, int count);
	bool loadFromFile(const string &vertPath, const string &fragPath);
	bool loadFromFile(const string &vertPath, const string &fragPath, const string &geomPath);

	/* 
	Load shaders from files with names <baseName>.<ext>, where ext is any of the following:
	.vs, .fs, .gs, .cs.
	*/
	bool loadFromFile(const string &baseName);

	/* 
	Load shaders from files with names <baseName>.<ext>, where ext is any of the following: .vs, .fs, .gs, .cs.,
	and performs shader compilation and linking into a program.
	*/
	bool loadAndLinkFromFile(const string &baseName);

	bool linkAndCheckStatus();
	void dispose();

	void begin();
	void end();

	/*
	Binds the out variable of the given name to the given color number. This can either be done
	in-shader, by using layout (location = x) out vec4 diffusecolor,
	automatically, by letting OpenGL assign an arbitrary color number, or
	through pre-link specification, using this function before linking the program.
	*/
	void bindFragDataLocation(const string &name, GLuint colorNumber);

	// Bind the attribute 'name' to 'location'
	void bindAttribute(GLuint location, const string &name);

	// Returns the cached uniform location
	GLint getUniformLocation(const string &name);

	// Returns the cached attribute location
	GLint getAttributeLocation(const string &name);

	GLuint getHandle() const;

	/*
	stride: number of components between the first element in each attribute
	offset: number of components into the array the component is found
	note: not byte offset
	*/
	void setAttributefv(const string &name, GLsizei numComponents, GLsizei stride, GLsizei offset);
	void setAttributefv(GLint location, GLsizei numComponents, GLsizei stride, GLsizei offset);
	void unsetAttribute(const string &name);
	void setAttributeDivisor(const string &name, GLuint divisor);
	/*void unsetAttribute(const std::string &name);
	void unsetAttribute(GLint location);*/

	void setUniform(const string &name, const mat4 &mat);
	void setUniform(const string &name, const mat3 &mat);
	void setUniform(const string &name, const mat2 &mat);
	void setUniform(const string &name, const vec4 &vec);
	void setUniform(const string &name, const vec3 &vec);
	void setUniform(const string &name, const vec2 &vec);
	void setUniform(const string &name, GLdouble d);
	void setUniform(const string &name, GLfloat f);
	void setUniform(const string &name, GLint i);
private:
	std::unordered_map<string, GLint> m_attrib_locs;
	std::unordered_map<string, GLint> m_uniform_locs;
	Program m_program;
	std::vector<Shader> m_shaders;
};

#endif