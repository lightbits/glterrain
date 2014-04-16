#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H
#include <gl/program.h>
#include <gl/shader.h>

class ShaderProgram
{
public:
	ShaderProgram();
	bool loadFromSource(const std::string &vertSrc, const std::string &fragSrc);
	bool loadFromFile(const std::string &vertName, const std::string &fragName);

	// Load shaders from files with names <baseName>.vs and <baseName>.fs
	bool loadFromFile(const std::string &baseName);

	// Load shaders from files with names <baseName>.vs and <baseName>.fs
	// and performs shader compilation and linking into a program.
	bool loadAndLinkFromFile(const std::string &baseName);

	bool linkAndCheckStatus();
	void dispose();

	void begin();
	void end();

	// Bind a user-defined varying out variable to a fragment shader color number.
	// 'colorNumber' must be less than GL_MAX_DRAW_BUFFERS.
	// The binding will have no effect until the program is next linked.
	void bindFragDataLocation(const std::string &name, GLuint colorNumber);

	void bindAttribute(GLuint location, const std::string &name);

	// Returns (a cached) uniform location for the given name
	GLint getUniformLocation(const std::string &name);

	// Returns (a cached) attribute location for the given name
	GLint getAttributeLocation(const std::string &name);

	/*
	stride: number of components between the first element in each attribute
	offset: number of components into the array the component is found
	note: not byte offset
	*/
	void setAttributefv(const std::string &name, GLsizei numComponents, GLsizei stride, GLsizei offset);
	void setAttributefv(GLint location, GLsizei numComponents, GLsizei stride, GLsizei offset);
	/*void unsetAttribute(const std::string &name);
	void unsetAttribute(GLint location);*/

	void setUniform(const std::string &name, const mat4 &mat);
	void setUniform(const std::string &name, const mat3 &mat);
	void setUniform(const std::string &name, const mat2 &mat);
	void setUniform(const std::string &name, const vec4 &vec);
	void setUniform(const std::string &name, const vec3 &vec);
	void setUniform(const std::string &name, const vec2 &vec);
	void setUniform(const std::string &name, GLdouble d);
	void setUniform(const std::string &name, GLfloat f);
	void setUniform(const std::string &name, GLint i);

	GLuint getProgram() const;
	GLuint getShader(GLenum type) const;
private:
	std::unordered_map<std::string, GLint> attribLocations;
	std::unordered_map<std::string, GLint> uniformLocations;
	Program program;
	Shader vertexShader;
	Shader fragmentShader;
};

#endif